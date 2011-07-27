/* x11type/x11type.cc
 *
 * X11Type
 *
 * Author: Thomas Habets <habets@google.com>
 *
 * Virtually type some keys in X11.
 *
 * http://code.google.com/p/x11type/
 * http://github.com/ThomasHabets/x11type/
 *
 */
/*
 * Copyright 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#ifndef BEGIN_NAMESPACE
#define BEGIN_NAMESPACE(x) namespace x {
#define END_NAMESPACE(x) }
#endif

BEGIN_NAMESPACE();
std::string argv0;
int verbose = 0;


/**
 * std::for_each(str.begin(), str.end(), Typer(display));
 */
class Typer {
        Display *display;
        Window winRoot;
        Window window;

        XKeyEvent createKeyEvent(bool press, int keycode, int modifiers);
public:
        Typer(Display *display, Window window);
        ~Typer();

        void operator()(char ch);
};


/**
 *
 */
Typer::Typer(Display *display, Window inwin)
        :display(display),window(inwin)
{
        winRoot = XDefaultRootWindow(display);
        if (!window) {
                int revert;
                if (verbose > 1) {
                        fprintf(stderr, "%s: locating window with focus\n",
                                argv0.c_str());
                }
                XGetInputFocus(display, &window, &revert);
        }

        if (verbose) {
                fprintf(stderr, "%s: root=0x%x focus=0x%x\n",
                        argv0.c_str(), winRoot, window);
        }
}


/**
 *
 */
Typer::~Typer()
{
        XFlush(display);
}


/**
 *
 */
void
printVersion()
{
        printf("x11type %s, by Thomas Habets <habets@google.com>\n"
               "Copyright (C) 2011 Google Inc\n"
               "License GPLv2: GNU GPL version 2 or later "
               "<http://gnu.org/licenses/gpl-2.0.html>\n"
               "This is free software: you are free to change and "
               "redistribute it.\n"
               "There is NO WARRANTY, to the extent permitted by law.\n",
               VERSION);
        exit(EXIT_SUCCESS);
}


/**
 *
 */
void
usage(int err)
{
        printf("x11type %s, by Thomas Habets <habets@google.com>\n"
               "Usage: %s [ -hvV ] [ -d <display> ] [ -w <win id> ]"
               " [ <text> ]\n"
               "\n"
               "\t-d <display>     Select display. Default to $DISPLAY\n"
               "\t-h, --help       Show this help text.\n"
               "\t-v               Increase verbosity.\n"
               "\t-V, --version    Show version.\n"
               "\t-w <window id>   Target specific window instead of active"
               " one.\n"
               "\n"
               "Report bugs to: habets@google.com\n"
               "X11Type Home:   <http://code.google.com/p/x11type/>\n"
               "X11Type Github: <http://github.com/ThomasHabets/x11type/>\n"
               , VERSION, argv0.c_str());
        exit(err);
}


/**
 *
 */
XKeyEvent
Typer::createKeyEvent(bool press, int keycode, int modifiers)
{
        XKeyEvent event;

        event.display     = display;
        event.window      = window;
        event.root        = winRoot;
        event.subwindow   = None;
        event.time        = CurrentTime;
        event.x           = 1;
        event.y           = 1;
        event.x_root      = 1;
        event.y_root      = 1;
        event.same_screen = True;
        event.keycode     = XKeysymToKeycode(display, keycode);
        event.state       = modifiers;

        event.type = press ? KeyPress : KeyRelease;

        return event;
}


/**
 *
 */
void
Typer::operator()(char ch_in)
{
        int ch(ch_in);

        if (verbose > 1) {
                fprintf(stderr, "%s: typing char <%c>\n", argv0.c_str(), ch);
        }
        if (ch == '\n') {
                ch = XK_Return;
        }
        XKeyEvent event = createKeyEvent(true, ch, 0);
        XSendEvent(event.display,
                   event.window,
                   True,
                   KeyPressMask,
                   (XEvent *)&event);

        // Send a fake key release event to the window.
        event = createKeyEvent(false, ch, 0);
        XSendEvent(event.display,
                   event.window,
                   True,
                   KeyPressMask,
                   (XEvent *)&event);
}


/**
 *
 */
void
typeString(Display *display, Window window, const std::string &str)
{
        std::for_each(str.begin(), str.end(), Typer(display, window));
}


/**
 *
 */
int
streamFile(Display *display, Window window, int fd)
{
        char buf[1024];
        ssize_t n;
        for (;;) {
                switch (n = read(fd, buf, sizeof(buf))) {
                case 0: // EOF
                        return EXIT_SUCCESS;
                case -1:
                        perror("read()");
                        return EXIT_FAILURE;
                default:
                        typeString(display, window,
                                   std::string(&buf[0], &buf[n]));
                }
        }
}

END_NAMESPACE();


/**
 *
 */
int
main(int argc, char **argv)
{
        argv0 = argv[0];
        const char *display_str = NULL;
        const char *str_arg = NULL;
        Window window = 0;

        { // handle GNU options
                int c;
                for (c = 1; c < argc; c++) {
                        if (!strcmp(argv[c], "--")) {
                                break;
                        } else if (!strcmp(argv[c], "--help")) {
                                usage(EXIT_SUCCESS);
                        } else if (!strcmp(argv[c], "--version")) {
                                printVersion();
                        }
                }
        }

        // option parsing
        int opt;
        while ((opt = getopt(argc, argv, "hd:vVw:")) != -1) {
                switch (opt) {
                case 'd':
                        display_str = optarg;
                        break;
                case 'h':
                        usage(EXIT_SUCCESS);
                        break;
                case 'v':
                        verbose++;
                        break;
                case 'V':
                        printVersion();
                        break;
                case 'w':
                        window = strtoul(optarg, 0, 0);
                        break;
                default: /* '?' */
                        usage(EXIT_FAILURE);
                }
        }
        if (optind < argc) {
                str_arg = argv[optind];
        }

        Display *display(XOpenDisplay(display_str));
        if (!display) {
                fprintf(stderr, "%s: Can't open display: %s\n",
                        argv0.c_str(), display_str ? display_str : "null");
                return EXIT_FAILURE;
        }
        if (str_arg) {
                // String supplied on command line, use that.
                typeString(display, window, str_arg);
                return EXIT_SUCCESS;
        } else {
                // No string on command line, stream all of stdin.
                return streamFile(display, window, STDIN_FILENO);
        }
        XCloseDisplay(display);
}
