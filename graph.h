#ifndef GRAPH_H
#define GRAPH_H

#include <stdlib.h>
#include <vector>
#include <gl\gl.h>

#include <iostream> //temp xxxxxxxxxxxxxxxxx

using namespace std;

struct graph_point
{
    float x,y,rel_x,rel_y;
};

class graph
{
    public:
        //Constructors
        graph();
        graph(int x_pos,int y_pos,int width,int height,bool dyn_x,bool dyn_y);//dynamic x, dynamic y
        graph(int x_pos,int y_pos,int width,int height,bool dyn_x,float min_y,float max_y);//dynamic x, min value for y, max value for y
        graph(int x_pos,int y_pos,int width,int height,float window_size_x,float min_y,float max_y);//window size of x, min value for y, max value for y
        graph(int x_pos,int y_pos,int width,int height,float window_size_x,bool dyn_y);//window size of x, dynamic y
        //Variables
        bool m_ready;
        //Functions
        bool add_point(float x);
        bool add_point(float x,float y);
        bool add_point_without_calc(float x,float y);
        bool force_pre_draw_calc(void);//have to be called if add_point_without_calc() was used
        bool set_color(float r,float g, float b,float a);
        int  get_status(void);
        bool set_status(int status);
        bool clean_up(int cut_value);
        bool set_view_x(bool dyn_x,int window_size=100);
        bool set_view_y(bool dyn_y,int y_min=0,int y_max=1);
        bool set_view_window(float extremes[4]);
        bool get_view_window(float extremes[4]);
        bool get_y_extremes(float extremes[4],float x_box[2]);
        bool draw_graph(void);

    private:
        //Variables
        int   m_status;// 0 = hide, 1 = less transparent, 2 = more transparent, 3 = show
        bool  m_dyn_x,m_dyn_y;
        int   m_x_pos,m_y_pos,m_width,m_height;//screen pos
        float m_max_x,m_max_y,m_min_x,m_min_y;//extreme values for values in store
        float m_window_size_x,m_window_y_min,m_window_y_max;//values for window size
        float m_color[4];
        vector<graph_point> m_vPoints;//stored values, in x order
        //Functions
        bool  pre_draw_calc(void);
};

#endif // GRAPH_H
