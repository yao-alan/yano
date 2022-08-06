#ifndef WINDOWING_H
#define WINDOWING_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <X11/Xlib.h>

const int32_t DEFAULT_WIDTH       = 800;
const int32_t DEFAULT_HEIGHT      = 600;
const char*   DEFAULT_WINDOW_NAME = "Generic Window";
const int16_t DEFAULT_DISPLAY_NUM = 0;
const uint8_t IMAGE_DEPTH         = 24; // "True Color (8-bit)"
const uint8_t IMAGE_STORAGE_DEPTH = 32;

/* Design of pw::Window:
    1. Instantiate with Window(width, height, title, display_num);
    2. Open a while(!shouldClose()) loop
    3. Add main logic into the loop, along with a pollEvents() call
    4. Two-part drawing procedure:
        a. Directly draw to drawable
        b. Refresh on-screen image
*/

namespace pw
{
    class Window 
    {
        public:
            // empty parameters: -1, -1, NULL, -1
            Window(int16_t     windowWidth,    // width in px
                   int16_t     windowHeight,   // height in px
                   const char *windowName,     // window title 
                   int16_t     displayNum);    // which monitor to put window on (0 is main)
            ~Window();

            // event functions
            bool shouldClose();
            void pollEvents(xcb_keycode_t *keycode);

            void display(int16_t x,
                         int16_t y,
                         uint16_t rows,
                         uint16_t cols);

            uint8_t            *drawable;      // format [width][height][4]
            uint16_t            window_width;
            uint16_t            window_height;
        private:
            // XCB-specific members
            xcb_connection_t          *m_connection;
            const xcb_setup_t         *m_setup;          // setup info
            xcb_window_t               m_xid;            // window id
            xcb_gcontext_t             m_gcid;           // graphics context id
            xcb_pixmap_t               m_pxid;           // pixmap id
            xcb_format_t              *m_pxfmt;          // pixmap format
            xcb_colormap_t             m_colormap;
            xcb_image_t               *m_image;

            // Window-specific members
            bool                m_should_close = false;
            const char         *m_window_name;

            // drawable setup function
            // for whatever reason, will not draw unless width*height*4 < 2^24
            void format_drawable(uint16_t width, uint16_t height) {
                drawable = (uint8_t *)malloc(width * height * (IMAGE_STORAGE_DEPTH >> 3));
                m_image = xcb_image_create(width,
                                           height,
                                           XCB_IMAGE_FORMAT_Z_PIXMAP,
                                           m_pxfmt->scanline_pad,
                                           m_pxfmt->depth,
                                           m_pxfmt->bits_per_pixel,
                                           0,
                                           XCB_IMAGE_ORDER_MSB_FIRST,
                                           XCB_IMAGE_ORDER_LSB_FIRST,
                                           drawable,
                                           width*height*(IMAGE_STORAGE_DEPTH >> 3),
                                           drawable);
            }
            // format setup function
            xcb_format_t *find_px_format(const xcb_setup_t *setup) {
                xcb_format_t *px_it = xcb_setup_pixmap_formats(setup);
                xcb_format_t *it_end = px_it + xcb_setup_pixmap_formats_length(setup);
                for ( ; px_it != it_end; ++px_it) {
                    printf("Available pixmap format: scanline_pad: %d, depth: %d, bpp: %d\n", 
                            px_it->scanline_pad, px_it->depth, px_it->bits_per_pixel);
                    if ((px_it->depth == IMAGE_DEPTH) && (px_it->bits_per_pixel == IMAGE_STORAGE_DEPTH))
                        return px_it;
                }
                printf("No suitable pixmap format found. Ending.\n");
                exit(-1);
            }
    };
};

pw::Window::Window(int16_t     windowWidth, 
                   int16_t     windowHeight, 
                   const char *windowName, 
                   int16_t     displayNum)
{
    // change to default parameters if necessary
    window_width = (windowWidth == -1) ? DEFAULT_WIDTH : windowWidth;
    window_height = (windowHeight == -1) ? DEFAULT_HEIGHT : windowHeight;
    m_window_name = (windowName == NULL) ? DEFAULT_WINDOW_NAME : windowName;
    if (displayNum == -1) displayNum = DEFAULT_DISPLAY_NUM;

    printf("Window width, height: (%d, %d)", window_width, window_height);

    m_connection = xcb_connect(
        NULL,   // const char*, display name; can be NULL (defaults to $DISPLAY)
        NULL    // int*, screen number of connection; NULL if don't care
    );
    if (xcb_connection_has_error(m_connection)) {
        printf("Error: Cannot open window.\n");
        exit(1);
    }

    // check basic info about connection
    m_setup = xcb_get_setup(m_connection);
    xcb_screen_iterator_t display_it = xcb_setup_roots_iterator(m_setup);

    // run through windows on each display; .rem is the number of remaining displays
    for (uint16_t i = 0; i < displayNum; ++i)
        xcb_screen_next(&display_it);

    // display preliminary info
    xcb_screen_t *display = display_it.data;
    printf("\n");
    printf("Max display resolution: (%d, %d)\n", display->width_in_pixels, display->height_in_pixels);

    // create the window
    m_xid = xcb_generate_id(m_connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[2] = { display->black_pixel, XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_EXPOSURE };
    xcb_create_window(m_connection,
                      XCB_COPY_FROM_PARENT, // window depth
                      m_xid,
                      display->root,        // parent window
                      0, 0,                 // x, y
                      window_width, window_height,
                      0,                    // border width
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, // class
                      display->root_visual, // visual
                      mask, values);        // masks

    // display the window
    xcb_map_window(m_connection, m_xid);
    xcb_flush(m_connection);

    // allocate color map
    m_colormap = display->default_colormap;

    // generate pixmap
    m_pxid = xcb_generate_id(m_connection);
    xcb_create_pixmap(m_connection,
                      display->root_depth,
                      m_pxid,
                      m_xid,
                      window_width,
                      window_height);

    // set up graphics context
    m_gcid = xcb_generate_id(m_connection);
    uint32_t valuemask = (XCB_GC_FOREGROUND | XCB_GC_BACKGROUND);
    const uint32_t valuelist[] = { display->black_pixel, display->white_pixel };
    xcb_create_gc(m_connection,
                  m_gcid,
                  m_pxid,
                  valuemask,
                  valuelist);

    // generate framebuffer
    m_pxfmt = find_px_format(m_setup);

    // create the drawable array attached to the window
    format_drawable(windowWidth, windowHeight);
    //format_drawable(display->width_in_pixels, display->height_in_pixels);

    // change name of window
    xcb_change_property(m_connection,
                        XCB_PROP_MODE_REPLACE,
                        m_xid,
                        XCB_ATOM_WM_NAME,
                        XCB_ATOM_STRING,
                        8,
                        strlen(m_window_name),
                        m_window_name);
    // supposed to set name of window icon; not working properly
    xcb_change_property(m_connection,
                        XCB_PROP_MODE_REPLACE,
                        m_xid,
                        XCB_ATOM_WM_ICON_NAME,
                        XCB_ATOM_STRING,
                        8,
                        strlen(m_window_name),
                        m_window_name);
}

pw::Window::~Window()
{
    free(m_image);
    free(drawable);
    xcb_free_pixmap(m_connection, m_pxid);
    xcb_free_colormap(m_connection, m_colormap);
    xcb_disconnect(m_connection);
    printf("Destroyed window.\n");
}

bool 
pw::Window::shouldClose() 
{
    return m_should_close;
}

void 
pw::Window::pollEvents(xcb_keycode_t *keycode)
{
    xcb_generic_event_t *event = xcb_poll_for_event(m_connection);
    if (event == NULL) return;
    switch (event->response_type & ~0x80) {
        case XCB_EXPOSE: {
            xcb_expose_event_t *expose = (xcb_expose_event_t *) event;
            window_width = expose->x + expose->width;
            window_height = expose->y + expose->height;
            display(expose->x, expose->y, expose->width, expose->height);
        }
        case XCB_KEY_PRESS: {
            xcb_key_press_event_t *kp = (xcb_key_press_event_t *)event;
            printf("%u\n", kp->state);
            printf("Key pressed: %u\n", kp->detail);
            if (kp->detail == 9) {
                printf("Closing window.\n");
                m_should_close = true;
            } else {
                *keycode = kp->detail;
            }
            break;
        }
    }
    //xcb_flush(m_connection);
    free(event);
}

void
pw::Window::display(int16_t x,
                    int16_t y,
                    uint16_t width,
                    uint16_t height)
{
    xcb_image_put(m_connection, m_pxid, m_gcid, m_image, 0, 0, 0);
    xcb_copy_area(m_connection, m_pxid, m_xid, m_gcid, x, y, x, y, width, height);
    xcb_flush(m_connection);    // image doesn't display unless this is written in
}

#endif