#ifndef YANO_H
#define YANO_H

#include <filesystem>
#include <fstream>
#include <list>
#include <string>
#include <vector>
#include <iostream>
#include <unistd.h>

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

            void keyHandler(xcb_keycode_t keycode);
        private:
            pw::Window                            *m_window;
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

            void drawGlyph(int row, int col, int scale, uint8_t keycode) {
                int xoffset = scale * col * m_glyph_properties.global_bbox_w;
                int yoffset = scale * row * m_glyph_properties.global_bbox_h;

                printf("%d, %d\n", xoffset, m_glyph_properties.global_bbox_w);

                int dp = (IMAGE_STORAGE_DEPTH >> 3);
                int p = (yoffset * m_window->window_width + xoffset) * dp;

                // string of 0s and 1s
                for (std::string &bitline : m_glyphs[keycode]) {
                    int p1 = p;
                    for (int c = 0; c < bitline.size(); ++c) {
                        int p2 = p;
                        if (bitline[c] == '1') {
                            for (int i = 0; i < scale; ++i) {
                                for (int j = 0; j < scale; ++j) {
                                    m_window->drawable[p+j*dp+0] = 255; // red
                                    m_window->drawable[p+j*dp+1] = 255; // green
                                    m_window->drawable[p+j*dp+2] = 255; // blue
                                }
                                p += m_window->window_width*dp;
                            }
                        } else {
                            for (int i = 0; i < scale; ++i) {
                                for (int j = 0; j < scale; ++j) {
                                    m_window->drawable[p+j*dp+0] = 0;
                                    m_window->drawable[p+j*dp+1] = 0;
                                    m_window->drawable[p+j*dp+2] = 0;
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
    m_font_scale = fontScale;
    m_keycode_table = new XToAscii();

    // load glyphs
    std::string fontDir = "./config/.glyphs/" + font;
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
    while (!m_window->shouldClose()) {
        xcb_keycode_t keycode;
        m_window->pollEvents(&keycode);
        keyHandler(keycode);
        keycode = 0;
        usleep(100000/6);
    }

    return 0;
}

void
yano::Yano::keyHandler(xcb_keycode_t keycode) {
    switch (keycode) {
        case 0: break;
        default: {
            char ascii_char = m_keycode_table->convert(keycode, 0);
            m_text_buffer.addChar(ascii_char);
            drawGlyph(m_text_buffer.m_cursor_position.row_coord,
                      m_text_buffer.m_cursor_position.col_coord,
                      m_font_scale,
                      ascii_char);
            m_window->display(0, 0, m_window->window_width, m_window->window_height);
            break;
        }
    }
}

#endif