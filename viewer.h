#ifndef VIEWER_H
#define VIEWER_H

#define _disp_width 50
#define _disp_width_text 100
#define _disp_height 20
#define _zoom_pix_min 20

#include <SOIL/SOIL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <math.h>
#include <gl/gl.h>
#include "graph.h"
#include "display.h"

using namespace std;

struct st_data_point
{
    st_data_point()
    {
        x=y=0;
    }
    st_data_point(float x_,float y_)
    {
        x=x_;
        y=y_;
    }

    float x,y;
};

struct st_line_data
{
    st_line_data()
    {
        pos_x_start=pos_x_end=pos_y=0;
    }
    st_line_data(float x_start,float x_end,float y_)
    {
        pos_x_start=x_start;
        pos_x_end=x_end;
        pos_y=y_;
    }

    float pos_x_start,pos_x_end,pos_y;
};

struct st_peptide
{
    st_peptide()
    {
        mass=0.0;
    }
    st_peptide(char aa,float _mass)
    {
        mass=0.0;
        sequence=string(1,aa);
    }
    st_peptide(string seq,float _mass)
    {
        mass=_mass;
        sequence=seq;
    }
    string sequence;
    float mass;
};

class viewer
{
    public:
        viewer();

        bool init(int screen_size[2],bool keys[256],int mouse_pos[2],bool mouse_but[2],string input_path,string run_path);
        bool cycle(void);
        bool update(void);
        bool draw(void);

    private:

        float m_pix2Da,m_pix2y_scale;
        bool  m_draw_mode,m_zoom_mode,m_just_deleted,m_delete_possible;
        int   m_draw_xpos_start,m_draw_ypos_start,m_draw_xpos_curr;
        int   m_zoom_pos_start[2],m_zoom_pos_end[2];
        bool* pMouse_but;
        int*  pMouse_pos;
        bool* pKeys;
        int   m_text_texture[3];
        int   m_screen_size[2];
        vector<st_data_point> m_vec_data;
        vector<display> m_vec_displays;
        vector<display> m_vec_displays_aa;
        vector<display> m_vec_displays_spec_mass;
        vector<st_line_data> m_vec_lines;
        graph m_graph;

        string get_aa_by_mass(float mass);
        float  get_aa_mono_mass(char aa);
        char   get_aa_by_int(int index);//0-20
};

#endif // VIEWER_H
