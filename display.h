#ifndef DISPLAY_H
#define DISPLAY_H

#include <iostream>
//#include <iomanip>

#include <sstream> //for float to strign conversion
#include <stdio.h> //for char test
#include <ctype.h> //for char test
#include <string>
#include <gl\gl.h>

using namespace std;

class display
{
    public:
        //Constructors
        display();
        display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],float value);
        display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text);
        //Variables
        bool m_ready;
        //Functions
        bool   set_pos(float new_pos[2]);
        bool   get_pos(float curr_pos[2]);
        bool   set_value(float value);
        float  get_value(void);
        bool   add_value(float value);
        bool   set_text(string text);
        string get_text(void);
        bool   draw_display(void);
        bool   setting_flags(bool border);
        bool   setting_flags(bool border,float colorRGB[3]);

    private:
        //Variables
        int    m_x_pos,m_y_pos,m_width,m_height;//screen pos
        int    m_max_char;
        bool   m_show_text,m_show_value,m_bright_font,m_show_border;
        string m_text;
        float  m_value;
        float  m_char_plate[12];
        float  m_char_width;
        float  m_border_color[3];
        int    m_font_texture[3];
        int    m_text_align;//1 left, 2 right, 3 center
        //Functions
        bool   calc_text_scale(void);
        bool   draw_letters_left(string text);
        bool   draw_letters_right(string text);
        bool   draw_letters_center(string text);
};

#endif // DISPLAY_H
