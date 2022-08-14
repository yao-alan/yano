#ifndef YANO_H
#define YANO_H

#include <filesystem>
#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <thread>

#include "windowing.h"
#include "xkeycodes.h"

// note that XCB cannot accept resolutions (W, H) where WxH >= 2^22.
// Limit window creation to at most 2560x1600.
namespace yano
{
    class Yano
    {
        public:
            Yano(int16_t     windowWidth,
                 int16_t     windowHeight,
                 std::string font,
                 uint8_t     fontScale);
            ~Yano();
            int run();

            void redraw() {
                while (!m_window->shouldClose()) {
                    m_window->display(0, 0, m_window->window_width, m_window->window_height);
                    usleep(100000/m_refresh_rate);
                }
            }

        private:
            pw::Window                            *m_window;
            uint8_t                                m_refresh_rate;
            uint8_t                                m_font_scale;
            std::vector<std::vector<std::string>>  m_glyphs;
            XToAscii                              *m_keycode_table;

            typedef struct glyphProperties {
                uint8_t global_bbox_w;
                uint8_t global_bbox_h;
                uint8_t global_xoff;
                uint8_t global_yoff;
            } glyphProperties;

            glyphProperties                        m_glyph_properties;

            void keyHandler(xcb_keycode_t keycode,
                            uint32_t     *modifiers);

            void drawGlyph(int row, int col, int scale, uint8_t keycode) {
                int xoffset = scale * col * m_glyph_properties.global_bbox_w;
                int yoffset = scale * row * m_glyph_properties.global_bbox_h;

                printf("%d, %d\n", xoffset, m_glyph_properties.global_bbox_w);

                int dp = (IMAGE_STORAGE_DEPTH >> 3);
                int p = (yoffset * m_window->window_width + xoffset) * dp;

                // string of 0s and 1s
                for (std::string &bitline : m_glyphs[keycode]) {
                    int p1 = p;
                    for (int c = 0; c < (int)bitline.size(); ++c) {
                        int p2 = p;
                        if (bitline[c] == '1') {
                            for (int i = 0; i < scale; ++i) {
                                for (int j = 0; j < scale; ++j) {
                                    m_window->drawable[p+j*dp+0] = 172; //244; // red
                                    m_window->drawable[p+j*dp+1] = 129; //239; // green
                                    m_window->drawable[p+j*dp+2] = 94; //236; // blue
                                }
                                p += m_window->window_width*dp;
                            }
                        } else {
                            for (int i = 0; i < scale; ++i) {
                                for (int j = 0; j < scale; ++j) {
                                    m_window->drawable[p+j*dp+0] = 64; //172;
                                    m_window->drawable[p+j*dp+1] = 52; //129;
                                    m_window->drawable[p+j*dp+2] = 46; //94;
                                }
                                p += m_window->window_width*dp;
                            }
                        }
                        p = p2 + scale * dp;
                    }
                    p = p1 + scale * m_window->window_width*dp;
                }
            }

            class TextBuffer
            {
                public:
                    TextBuffer() {
                        m_lines.push_back(std::list<char>());
                        m_cursor_position.curr_row = m_lines.begin();
                        m_cursor_position.curr_col = m_cursor_position.curr_row->begin();

                        m_cursor_position.row_coord = 0;
                        m_cursor_position.col_coord = 0;
                    }

                    void addChar(char ch) {
                        // if iterator is at end of current line
                        if (m_cursor_position.curr_row->size() == 0 ||
                            next(m_cursor_position.curr_col) == m_cursor_position.curr_row->end())
                        {
                            m_cursor_position.curr_row->push_back(ch);
                            m_cursor_position.curr_col = prev(m_cursor_position.curr_row->end());
                            m_cursor_position.col_coord++;
                            if (ch == '\n') {
                                m_lines.push_back(std::list<char>());
                                m_cursor_position.curr_row++;
                                m_cursor_position.curr_col = m_cursor_position.curr_row->begin();
                                m_cursor_position.row_coord++;
                                m_cursor_position.col_coord = 0;
                            }
                        } else {
                            m_cursor_position.curr_row->insert(next(m_cursor_position.curr_col), ch);
                            m_cursor_position.curr_col++;
                            m_cursor_position.col_coord++;
                            // split the current line
                            if (ch == '\n') {
                                auto newPos = m_lines.insert(next(m_cursor_position.curr_row), std::list<char>());
                                newPos->splice(newPos->begin(), *m_cursor_position.curr_row,
                                               m_cursor_position.curr_col, m_cursor_position.curr_row->end());
                                m_cursor_position.curr_row->erase(m_cursor_position.curr_col, m_cursor_position.curr_row->end());
                                m_cursor_position.curr_col = newPos->begin();
                                m_cursor_position.row_coord++;
                                m_cursor_position.col_coord = 0;
                            }
                        }
                    }

                    void delChar() {
                        // if iterator is at beginning of current line
                        if (m_cursor_position.curr_col == m_cursor_position.curr_row->end()) {
                            // can only delete if not at beginning of file
                            if (m_cursor_position.curr_row != m_lines.begin()) {
                                m_cursor_position.curr_row--;
                                m_lines.erase(next(m_cursor_position.curr_row));
                                // erase the '\n'
                                m_cursor_position.curr_row->erase(prev(m_cursor_position.curr_row->end()));
                                m_cursor_position.curr_col = prev(m_cursor_position.curr_row->end());
                                m_cursor_position.row_coord--;
                                m_cursor_position.col_coord = m_cursor_position.curr_row->size();
                            }
                        } else {
                            m_cursor_position.curr_col--;
                            m_cursor_position.curr_row->erase(next(m_cursor_position.curr_col));
                            m_cursor_position.col_coord--;
                        }
                    }

                    std::list<std::list<char>> m_lines;

                    typedef struct CursorPosition {
                        std::list<std::list<char>>::iterator curr_row;
                        std::list<char>::iterator curr_col;
                        // added here for easy reference by Yano
                        int row_coord;
                        int col_coord;
                    } CursorPosition;

                    CursorPosition m_cursor_position;
            };

            TextBuffer                             m_text_buffer;
    };
};

yano::Yano::Yano(int16_t windowWidth,
                 int16_t windowHeight,
                 std::string font,
                 uint8_t fontScale)
{
    m_window = new pw::Window(windowWidth, windowHeight, "yano", -1);
    m_refresh_rate = 60;
    m_font_scale = fontScale;
    m_keycode_table = new XToAscii();

    // change background color
    int bits_per_px = IMAGE_STORAGE_DEPTH >> 3;
    for (int i = 0; i < windowWidth*windowHeight*bits_per_px; i += bits_per_px) {
        m_window->drawable[i+0] = 64;
        m_window->drawable[i+1] = 52;
        m_window->drawable[i+2] = 46;
    }

    // load glyphs
    std::string fontDir = "../config/.glyphs/" + font;
    for (const auto &entry : std::filesystem::directory_iterator(fontDir)) {
        std::fstream glyphFile;
        glyphFile.open(entry.path());

        if (glyphFile.is_open()) {
            std::string global_bbox_w, global_bbox_h, global_xoff, global_yoff;
            std::string glyph_bbox_w, glyph_bbox_h, glyph_xoff, glyph_yoff;

            glyphFile >> global_bbox_w >> global_bbox_h >> global_xoff >> global_yoff;
            glyphFile >> glyph_bbox_w >> glyph_bbox_h >> glyph_xoff >> glyph_yoff;

            m_glyph_properties.global_bbox_w = std::stoi(global_bbox_w);
            m_glyph_properties.global_bbox_h = std::stoi(global_bbox_h);
            m_glyph_properties.global_xoff = std::stoi(global_xoff);
            m_glyph_properties.global_yoff = std::stoi(global_yoff);

            glyphFile.ignore(1000, '\n');

            m_glyphs.push_back(std::vector<std::string>());

            std::string bitline;
            while (getline(glyphFile, bitline)) {
                m_glyphs.back().push_back(bitline);
            }

            glyphFile.close();
        }
    }
}

yano::Yano::~Yano() {
    delete(m_keycode_table);
    delete(m_window);
}

int
yano::Yano::run() {
    std::thread render(&yano::Yano::redraw, this);

    while (!m_window->shouldClose()) {
        xcb_keycode_t keycode;
        uint32_t modifiers[4] = {0, 0, 0, 0}; // SHIFT, LOCK, CTRL, ALT
        m_window->pollEvents(&keycode, modifiers);
        keyHandler(keycode, modifiers);

        // draw cursor
        //drawGlyph(m_text_buffer.m_cursor_position.row_coord,
        //            m_text_buffer.m_cursor_position.col_coord,
        //            m_font_scale,
        //            ascii_char);

        keycode = 0;
        usleep(100000/m_refresh_rate);
    }

    render.join();

    return 0;
}

void
yano::Yano::keyHandler(xcb_keycode_t keycode, uint32_t *modifiers) {
    switch (keycode) {
        case 0: break;
        // backspace
        case BACKSPACE: {
            m_text_buffer.delChar();
            drawGlyph(m_text_buffer.m_cursor_position.row_coord,
                      m_text_buffer.m_cursor_position.col_coord,
                      m_font_scale,
                      0); // draw over with a null char
            break;
        }
        default: {
            char ascii_char = m_keycode_table->convert(keycode, modifiers[0] & 1);
            m_text_buffer.addChar(ascii_char);
            drawGlyph(m_text_buffer.m_cursor_position.row_coord,
                      m_text_buffer.m_cursor_position.col_coord,
                      m_font_scale,
                      ascii_char);
            break;
        }
    }
}

#endif