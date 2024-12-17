/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * This file is part of the TinyUSB stack.
 */

#include "tusb_option.h"

#if (CFG_TUD_ENABLED && CFG_TUD_HID)

//--------------------------------------------------------------------+
// INCLUDE
//--------------------------------------------------------------------+
#include "device/usbd.h"
#include "device/usbd_pvt.h"

#include "hid_device.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
typedef struct {
  uint8_t itf_num;
  uint8_t ep_in;
  uint8_t ep_out;       // optional Out endpoint
  uint8_t itf_protocol; // Boot mouse or keyboard

  uint16_t report_desc_len;
  uint8_t protocol_mode; // Boot (0) or Report protocol (1)
  uint8_t idle_rate;     // up to application to handle idle rate

  // TODO save hid descriptor since host can specifically request this after enumeration
  // Note: HID descriptor may be not available from application after enumeration
  const tusb_hid_descriptor_hid_t*hid_descriptor;
} hidd_interface_t;

typedef struct {
  TUD_EPBUF_DEF(ctrl , CFG_TUD_HID_EP_BUFSIZE);
  TUD_EPBUF_DEF(epin , CFG_TUD_HID_EP_BUFSIZE);
  TUD_EPBUF_DEF(epout, CFG_TUD_HID_EP_BUFSIZE);
} hidd_epbuf_t;

static hidd_interface_t _hidd_itf[CFG_TUD_HID];
CFG_TUD_MEM_SECTION static hidd_epbuf_t _hidd_epbuf[CFG_TUD_HID];

/*------------- Helpers -------------*/
TU_ATTR_ALWAYS_INLINE static inline uint8_t get_index_by_itfnum(uint8_t itf_num) {
  for (uint8_t i = 0; i < CFG_TUD_HID; i++) {
    if (itf_num == _hidd_itf[i].itf_num) {
      return i;
    }
  }
  return 0xFF;
}

//--------------------------------------------------------------------+
// Weak stubs: invoked if no strong implementation is available
//--------------------------------------------------------------------+
TU_ATTR_WEAK void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol) {
  (void) instance;
  (void) protocol;
}

TU_ATTR_WEAK bool tud_hid_set_idle_cb(uint8_t instance, uint8_t idle_rate) {
  (void) instance;
  (void) idle_rate;
  return true;
}

TU_ATTR_WEAK void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len) {
  (void) instance;
  (void) report;
  (void) len;
}

// Invoked when a transfer wasn't successful
TU_ATTR_WEAK void tud_hid_report_failed_cb(uint8_t instance, hid_report_type_t report_type, uint8_t const* report, uint16_t xferred_bytes) {
  (void) instance;
  (void) report_type;
  (void) report;
  (void) xferred_bytes;
}

//--------------------------------------------------------------------+
// APPLICATION API
//--------------------------------------------------------------------+
bool tud_hid_n_ready(uint8_t instance) {
  uint8_t const rhport = 0;
  uint8_t const ep_in = _hidd_itf[instance].ep_in;
  return tud_ready() && (ep_in != 0) && !usbd_edpt_busy(rhport, ep_in);
}

bool tud_hid_n_report(uint8_t instance, uint8_t report_id, void const *report, uint16_t len) {
  TU_VERIFY(instance < CFG_TUD_HID);
  const uint8_t rhport = 0;
  hidd_interface_t *p_hid = &_hidd_itf[instance];
  hidd_epbuf_t *p_epbuf = &_hidd_epbuf[instance];

  // claim endpoint
  TU_VERIFY(usbd_edpt_claim(rhport, p_hid->ep_in));

  // prepare data
  if (report_id) {
    p_epbuf->epin[0] = report_id;
    TU_VERIFY(0 == tu_memcpy_s(p_epbuf->epin + 1, CFG_TUD_HID_EP_BUFSIZE - 1, report, len));
    len++;
  } else {
    TU_VERIFY(0 == tu_memcpy_s(p_epbuf->epin, CFG_TUD_HID_EP_BUFSIZE, report, len));
  }

  return usbd_edpt_xfer(rhport, p_hid->ep_in, p_epbuf->epin, len);
}

uint8_t tud_hid_n_interface_protocol(uint8_t instance) {
  return _hidd_itf[instance].itf_protocol;
}

uint8_t tud_hid_n_get_protocol(uint8_t instance) {
  return _hidd_itf[instance].protocol_mode;
}

bool tud_hid_n_keyboard_report(uint8_t instance, uint8_t report_id, uint8_t modifier, const uint8_t keycode[6]) {
  hid_keyboard_report_t report;
  report.modifier = modifier;
  report.reserved = 0;

  if (keycode) {
    memcpy(report.keycode, keycode, sizeof(report.keycode));
  } else {
    tu_memclr(report.keycode, 6);
  }

  return tud_hid_n_report(instance, report_id, &report, sizeof(report));
}

bool tud_hid_n_mouse_report(uint8_t instance, uint8_t report_id,
                            uint8_t buttons, int8_t x, int8_t y, int8_t vertical, int8_t horizontal) {
  hid_mouse_report_t report = {
    .buttons = buttons,
    .x = x,
    .y = y,
    .wheel = vertical,
    .pan = horizontal
  };

  return tud_hid_n_report(instance, report_id, &report, sizeof(report));
}

bool tud_hid_n_abs_mouse_report(uint8_t instance, uint8_t report_id,
                                uint8_t buttons, int16_t x, int16_t y, int8_t vertical, int8_t horizontal) {
  hid_abs_mouse_report_t report = {
    .buttons = buttons,
    .x = x,
    .y = y,
    .wheel = vertical,
    .pan = horizontal
  };
  return tud_hid_n_report(instance, report_id, &report, sizeof(report));
}

bool tud_hid_n_gamepad_report(uint8_t instance, uint8_t report_id,
                              int8_t x, int8_t y, int8_t z, int8_t rz, int8_t rx, int8_t ry, uint8_t hat, uint32_t buttons) {
  hid_gamepad_report_t report = {
      .x = x,
      .y = y,
      .z = z,
      .rz = rz,
      .rx = rx,
      .ry = ry,
      .hat = hat,
      .buttons = buttons,
  };

  return tud_hid_n_report(instance, report_id, &report, sizeof(report));
}

//--------------------------------------------------------------------+
// USBD-CLASS API
//--------------------------------------------------------------------+
void hidd_init(void) {
  hidd_reset(0);
}

bool hidd_deinit(void) {
  return true;
}

void hidd_reset(uint8_t rhport) {
  (void)rhport;
  tu_memclr(_hidd_itf, sizeof(_hidd_itf));
}

uint16_t hidd_open(uint8_t rhport, tusb_desc_interface_t const *desc_itf, uint16_t max_len) {
  TU_VERIFY(TUSB_CLASS_HID == desc_itf->bInterfaceClass, 0);

  // len = interface + hid + n*endpoints
  uint16_t const drv_len = (uint16_t) (sizeof(tusb_desc_interface_t) + sizeof(tusb_hid_descriptor_hid_t) +
                                       desc_itf->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
  TU_ASSERT(max_len >= drv_len, 0);

  // Find available interface
  hidd_interface_t *p_hid;
  uint8_t hid_id;
  for (hid_id = 0; hid_id < CFG_TUD_HID; hid_id++) {
    p_hid = &_hidd_itf[hid_id];
    if (p_hid->ep_in == 0) {
      break;
    }
  }
  TU_ASSERT(hid_id < CFG_TUD_HID, 0);
  hidd_epbuf_t *p_epbuf = &_hidd_epbuf[hid_id];

  uint8_t const *p_desc = (uint8_t const *)desc_itf;

  //------------- HID descriptor -------------//
  p_desc = tu_desc_next(p_desc);
  TU_ASSERT(HID_DESC_TYPE_HID == tu_desc_type(p_desc), 0);
  p_hid->hid_descriptor = (tusb_hid_descriptor_hid_t const *)p_desc;

  //------------- Endpoint Descriptor -------------//
  p_desc = tu_desc_next(p_desc);
  TU_ASSERT(usbd_open_edpt_pair(rhport, p_desc, desc_itf->bNumEndpoints, TUSB_XFER_INTERRUPT, &p_hid->ep_out, &p_hid->ep_in), 0);

  if (desc_itf->bInterfaceSubClass == HID_SUBCLASS_BOOT) {
    p_hid->itf_protocol = desc_itf->bInterfaceProtocol;
  }

  p_hid->protocol_mode = HID_PROTOCOL_REPORT; // Per Specs: default is report mode
  p_hid->itf_num = desc_itf->bInterfaceNumber;

  // Use offsetof to avoid pointer to the odd/misaligned address
  p_hid->report_desc_len = tu_unaligned_read16((uint8_t const *)p_hid->hid_descriptor + offsetof(tusb_hid_descriptor_hid_t, wReportLength));

  // Prepare for output endpoint
  if (p_hid->ep_out) {
    TU_ASSERT(usbd_edpt_xfer(rhport, p_hid->ep_out, p_epbuf->epout, CFG_TUD_HID_EP_BUFSIZE), drv_len);
  }

  return drv_len;
}

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage (setup/data/ack)
// return false to stall control endpoint (e.g unsupported request)
bool hidd_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
  TU_VERIFY(request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE);

  uint8_t const hid_itf = get_index_by_itfnum((uint8_t)request->wIndex);
  TU_VERIFY(hid_itf < CFG_TUD_HID);
  hidd_interface_t *p_hid = &_hidd_itf[hid_itf];
  hidd_epbuf_t *p_epbuf = &_hidd_epbuf[hid_itf];

  if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_STANDARD) {
    //------------- STD Request -------------//
    if (stage == CONTROL_STAGE_SETUP) {
      uint8_t const desc_type = tu_u16_high(request->wValue);
      // uint8_t const desc_index = tu_u16_low (request->wValue);

      if (request->bRequest == TUSB_REQ_GET_DESCRIPTOR && desc_type == HID_DESC_TYPE_HID) {
        TU_VERIFY(p_hid->hid_descriptor);
        TU_VERIFY(tud_control_xfer(rhport, request, (void *)(uintptr_t)p_hid->hid_descriptor, p_hid->hid_descriptor->bLength));
      } else if (request->bRequest == TUSB_REQ_GET_DESCRIPTOR && desc_type == HID_DESC_TYPE_REPORT) {
        uint8_t const *desc_report = tud_hid_descriptor_report_cb(hid_itf);
        tud_control_xfer(rhport, request, (void *)(uintptr_t)desc_report, p_hid->report_desc_len);
      } else {
        return false; // stall unsupported request
      }
    }
  } else if (request->bmRequestType_bit.type == TUSB_REQ_TYPE_CLASS) {
    //------------- Class Specific Request -------------//
    switch (request->bRequest) {
      case HID_REQ_CONTROL_GET_REPORT:
        if (stage == CONTROL_STAGE_SETUP) {
          uint8_t const report_type = tu_u16_high(request->wValue);
          uint8_t const report_id = tu_u16_low(request->wValue);

          uint8_t* report_buf = p_epbuf->ctrl;
          uint16_t req_len = tu_min16(request->wLength, CFG_TUD_HID_EP_BUFSIZE);
          uint16_t xferlen = 0;

          // If host request a specific Report ID, add ID to as 1 byte of response
          if ((report_id != HID_REPORT_TYPE_INVALID) && (req_len > 1)) {
            *report_buf++ = report_id;
            req_len--;
            xferlen++;
          }

          xferlen += tud_hid_get_report_cb(hid_itf, report_id, (hid_report_type_t) report_type, report_buf, req_len);
          TU_ASSERT(xferlen > 0);

          tud_control_xfer(rhport, request, p_epbuf->ctrl, xferlen);
        }
        break;

      case HID_REQ_CONTROL_SET_REPORT:
        if (stage == CONTROL_STAGE_SETUP) {
          TU_VERIFY(request->wLength <= CFG_TUD_HID_EP_BUFSIZE);
          tud_control_xfer(rhport, request, p_epbuf->ctrl, request->wLength);
        } else if (stage == CONTROL_STAGE_ACK) {
          uint8_t const report_type = tu_u16_high(request->wValue);
          uint8_t const report_id = tu_u16_low(request->wValue);

          uint8_t const* report_buf = p_epbuf->ctrl;
          uint16_t report_len = tu_min16(request->wLength, CFG_TUD_HID_EP_BUFSIZE);

          // If host request a specific Report ID, extract report ID in buffer before invoking callback
          if ((report_id != HID_REPORT_TYPE_INVALID) && (report_len > 1) && (report_id == report_buf[0])) {
            report_buf++;
            report_len--;
          }

          tud_hid_set_report_cb(hid_itf, report_id, (hid_report_type_t) report_type, report_buf, report_len);
        }
        break;

      case HID_REQ_CONTROL_SET_IDLE:
        if (stage == CONTROL_STAGE_SETUP) {
          p_hid->idle_rate = tu_u16_high(request->wValue);
          TU_VERIFY(tud_hid_set_idle_cb(hid_itf, p_hid->idle_rate)); // stall if false
          tud_control_status(rhport, request);
        }
        break;

      case HID_REQ_CONTROL_GET_IDLE:
        if (stage == CONTROL_STAGE_SETUP) {
          // TODO idle rate of report
          tud_control_xfer(rhport, request, &p_hid->idle_rate, 1);
        }
        break;

      case HID_REQ_CONTROL_GET_PROTOCOL:
        if (stage == CONTROL_STAGE_SETUP) {
          tud_control_xfer(rhport, request, &p_hid->protocol_mode, 1);
        }
        break;

      case HID_REQ_CONTROL_SET_PROTOCOL:
        if (stage == CONTROL_STAGE_SETUP) {
          tud_control_status(rhport, request);
        } else if (stage == CONTROL_STAGE_ACK) {
          p_hid->protocol_mode = (uint8_t) request->wValue;
          tud_hid_set_protocol_cb(hid_itf, p_hid->protocol_mode);
        }
        break;

      default:
        return false; // stall unsupported request
    }
  } else {
    return false; // stall unsupported request
  }

  return true;
}

bool hidd_xfer_cb(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
  uint8_t instance;
  hidd_interface_t *p_hid;

  // Identify which interface to use
  for (instance = 0; instance < CFG_TUD_HID; instance++) {
    p_hid = &_hidd_itf[instance];
    if ((ep_addr == p_hid->ep_out) || (ep_addr == p_hid->ep_in)) {
      break;
    }
  }
  TU_ASSERT(instance < CFG_TUD_HID);
  hidd_epbuf_t *p_epbuf = &_hidd_epbuf[instance];

  if (ep_addr == p_hid->ep_in) {
    // Input report
    if (XFER_RESULT_SUCCESS == result) {
      tud_hid_report_complete_cb(instance, p_epbuf->epin, (uint16_t) xferred_bytes);
    } else {
      tud_hid_report_failed_cb(instance, HID_REPORT_TYPE_INPUT, p_epbuf->epin, (uint16_t) xferred_bytes);
    }
  } else {
    // Output report
    if (XFER_RESULT_SUCCESS == result) {
      tud_hid_set_report_cb(instance, 0, HID_REPORT_TYPE_OUTPUT, p_epbuf->epout, (uint16_t)xferred_bytes);
    } else {
      tud_hid_report_failed_cb(instance, HID_REPORT_TYPE_OUTPUT, p_epbuf->epout, (uint16_t) xferred_bytes);
    }

    // prepare for new transfer
    TU_ASSERT(usbd_edpt_xfer(rhport, p_hid->ep_out, p_epbuf->epout, CFG_TUD_HID_EP_BUFSIZE));
  }

  return true;
}

#endif
