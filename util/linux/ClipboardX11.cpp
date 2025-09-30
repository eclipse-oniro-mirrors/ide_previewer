/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ClipboardX11.h"
#include <iostream>
#include <cstring>
#include <X11/Xlib.h>

void ClipboardX11::SetClipboardData(const std::string& str)
{
    display = XOpenDisplay(nullptr);
    int ret = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, ret), 0, 0, 1, 1, 0,
        BlackPixel(display, ret), WhitePixel(display, ret));
    targets_atom = XInternAtom(display, "TARGETS", 0);
    text_atom = XInternAtom(display, "TEXT", 0);
    UTF8 = XInternAtom(display, "UTF8_STRING", 1);
    if (UTF8 == None) {
        UTF8 = XA_STRING;
    }
    Atom selection = XInternAtom(display, "CLIPBOARD", 0);
    SetCopyData(selection, str, strlen(str.c_str()));
}

const std::string ClipboardX11::GetClipboardData()
{
    display = XOpenDisplay(nullptr);
    int ret = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, ret), 0, 0, 1, 1, 0,
                                 BlackPixel(display, ret), WhitePixel(display, ret));
    std::string retStr; // NOLINT
    UTF8 = XInternAtom(display, "UTF8_STRING", True);
    if (UTF8 != None) {
        retStr = GetPasteType(UTF8);
    }
    if (retStr.empty()) {
        retStr = GetPasteType(XA_STRING);
    }
    return retStr;
}

void ClipboardX11::SetCopyData(const Atom& selection, const std::string& text, const int size)
{
    XEvent event;
    XSetSelectionOwner (display, selection, window, 0);
    if (XGetSelectionOwner (display, selection) != window) {
        return;
    }
    while (1) {
        XNextEvent (display, &event);
        if (event.type == SelectionRequest) {
            if (event.xselectionrequest.selection != selection) {
                break;
            }
            XSelectionRequestEvent* xsr = &event.xselectionrequest;
            XSelectionEvent ev = {0};
            int ret = 0;
            ev.type = SelectionNotify, ev.display = xsr->display, ev.requestor = xsr->requestor,
            ev.selection = xsr->selection, ev.time = xsr->time, ev.target = xsr->target,
            ev.property = xsr->property;
            if (ev.target == targets_atom) {
                ret = XChangeProperty(ev.display, ev.requestor, ev.property,
                                      XA_ATOM, nFormateProp32, PropModeReplace,
                                      (unsigned char*)&UTF8, 1);
            } else if (ev.target == XA_STRING || ev.target == text_atom) {
                ret = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING,
                                      nFormateProp8, PropModeReplace, (unsigned char*)text.c_str(), size);
            } else if (ev.target == UTF8) {
                ret = XChangeProperty(ev.display, ev.requestor, ev.property, UTF8,
                                      nFormateProp8, PropModeReplace, (unsigned char*)text.c_str(), size);
            } else {
                ev.property = None;
            }
            if ((ret & nEvenNumber) == 0) {
                XSendEvent (display, ev.requestor, 0, 0, (XEvent *)&ev);
            }
        }
    }
}

std::string ClipboardX11::GetPasteType(const Atom& atom)
{
    XEvent event;
    int format;
    unsigned long num;
    unsigned long size;
    char* data = nullptr;
    std::string retStr;
    Atom target, CLIPBOARD = XInternAtom(display, "CLIPBOARD", 0), XSEL_DATA = XInternAtom(display, "XSEL_DATA", 0);
    XConvertSelection(display, CLIPBOARD, atom, XSEL_DATA, window, CurrentTime);
    XSync(display, 0);
    XNextEvent(display, &event);
    if (event.type == SelectionNotify) {
        if (event.xselection.selection != CLIPBOARD) {
            return retStr;
        }
        if (event.xselection.property) {
            XGetWindowProperty(event.xselection.display, event.xselection.requestor, event.xselection.property, 0L,
                               (~0L), 0, AnyPropertyType, &target, &format, &size, &num, (unsigned char**)&data);
            if (target == UTF8 || target == XA_STRING) {
                std::string str(data);
                retStr = str;
            }
            XFree(data);
            XDeleteProperty(event.xselection.display, event.xselection.requestor, event.xselection.property);
        }
    }
    return retStr;
}