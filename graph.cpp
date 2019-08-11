#include "graph.h"

graph::graph()
{
    m_ready=false;
}

graph::graph(int x_pos,int y_pos,int width,int height,bool dyn_x,bool dyn_y)//dynamic x, dynamic y
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_dyn_x=true;
    m_dyn_y=true;

    m_status=1;
    m_color[0]=0; m_color[1]=1; m_color[2]=0; m_color[3]=1;

    m_ready=true;
}

graph::graph(int x_pos,int y_pos,int width,int height,bool dyn_x,float min_y,float max_y)//dynamic x, min value for y, max value for y
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_dyn_x=true;
    m_dyn_y=false;

    m_window_y_min=min_y;
    m_window_y_max=max_y;

    m_status=2;
    m_color[0]=0; m_color[1]=1; m_color[2]=0; m_color[3]=1;

    m_ready=true;
}

graph::graph(int x_pos,int y_pos,int width,int height,float window_size_x,float min_y,float max_y)//window size of x, min value for y, max value for y
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_dyn_x=false;
    m_dyn_y=false;

    m_window_size_x=window_size_x;
    m_window_y_min=min_y;
    m_window_y_max=max_y;

    m_status=2;
    m_color[0]=0; m_color[1]=1; m_color[2]=0; m_color[3]=1;

    m_ready=true;
}

graph::graph(int x_pos,int y_pos,int width,int height,float window_size_x,bool dyn_y)//window size of x, dynamic y
{
    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_dyn_x=false;
    m_dyn_y=true;

    m_window_size_x=window_size_x;

    m_status=2;
    m_color[0]=0; m_color[1]=1; m_color[2]=0; m_color[3]=1;

    m_ready=true;
}

bool graph::add_point(float x)//add random y value
{
    int range=int( (m_max_y-m_min_y)*10000 );
    float y= float(rand()%range)/10000 + m_min_y;
    return add_point(x,y);
}

bool graph::add_point(float x,float y)
{
    //if first point assign extremes
    if(m_vPoints.empty())
    {
        m_max_x=x+1;
        m_max_y=y+1;
        m_min_x=x-1;
        m_min_y=y-1;
    }

    graph_point temp_point;
    temp_point.x=x;
    temp_point.y=y;
    m_vPoints.push_back(temp_point);
    //is new point extreme?
    if(x>m_max_x) m_max_x=x;
    if(y>m_max_y) m_max_y=y;
    if(x<m_min_x) m_min_x=x;
    if(y<m_min_y) m_min_y=y;

    //check if old values should be removed
    if(!m_dyn_x)//check if values ar within window (x)
    {
        int highest_element_for_removal=-1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            if(m_vPoints[i].x<m_max_x-m_window_size_x)
            {
                highest_element_for_removal=i;
            }
            else
            {
                if(highest_element_for_removal>-1&&highest_element_for_removal<(int)m_vPoints.size())
                {
                    m_vPoints.erase(m_vPoints.begin(),m_vPoints.begin()+highest_element_for_removal);
                }
                break;//rest of points should be inside
            }
        }
    }
    //check for extremes
    if(!m_vPoints.empty())
    {
        //starting values
        float max_x=m_vPoints[0].x;
        float max_y=m_vPoints[0].y;
        float min_x=m_vPoints[0].x;
        float min_y=m_vPoints[0].y;
        //find extremes
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            if(m_vPoints[i].x>max_x) max_x=m_vPoints[i].x;
            if(m_vPoints[i].y>max_y) max_y=m_vPoints[i].y;
            if(m_vPoints[i].x<min_x) min_x=m_vPoints[i].x;
            if(m_vPoints[i].y<min_y) min_y=m_vPoints[i].y;
        }
        //assign new extremes
        m_max_x=max_x;
        m_max_y=max_y;
        m_min_x=min_x;
        m_min_y=min_y;
    }
    else//should not happen
    {
        m_max_x=1;m_min_x=-1;
        m_max_y=1;m_min_y=-1;
    }

    if(m_dyn_x)//clean up
    {
        if((int)m_vPoints.size()>100) clean_up(1);
    }

    pre_draw_calc();//necessary if changes in the graphs have been made

    return true;
}

bool graph::add_point_without_calc(float x,float y)
{
    graph_point temp_point;
    temp_point.x=x;
    temp_point.y=y;
    m_vPoints.push_back(temp_point);

    return true;
}

bool graph::force_pre_draw_calc(void)
{
    //find extremes
    if(m_vPoints.empty()) return false;
    m_max_x=m_vPoints[0].x;
    m_max_y=m_vPoints[0].y;
    m_min_x=m_vPoints[0].x;
    m_min_y=m_vPoints[0].y;
    for(int i=0;i<(int)m_vPoints.size();i++)
    {
        if(m_vPoints[i].x>m_max_x) m_max_x=m_vPoints[i].x;
        if(m_vPoints[i].y>m_max_y) m_max_y=m_vPoints[i].y;
        if(m_vPoints[i].x<m_min_x) m_min_x=m_vPoints[i].x;
        if(m_vPoints[i].y<m_min_y) m_min_y=m_vPoints[i].y;
    }

    //calc
    pre_draw_calc();

    return true;
}

bool graph::set_color(float r,float g, float b,float a)
{
    m_color[0]=r; m_color[1]=g; m_color[2]=b; m_color[3]=a;
    return true;
}

int graph::get_status(void)
{
    return m_status;
}

bool graph::set_status(int status)
{
    m_status=status;

    return true;
}

bool graph::pre_draw_calc(void)
{
    if(!m_ready) return false;
    //calc rel values:
    //if dyn y, normalize values from min y to max y, if not dyn, normalize from static values
    //if dyn x (draw all values), normalize from first x to last x, if not dyn, normalize from first x to last x (same, but diffrent vector size)
    //calc window pos from rel pos:
    //local min x = graph x pos, local max x = graph x pos + width
    //local min y = graph y pos - height, local max y = graph y pos

    if(m_dyn_y)
    {
        float range=m_max_y-m_min_y;
        if(range==0) range=1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            float rel_y=(m_vPoints[i].y-m_min_y)/range;
            float scaled_y=rel_y*float(-m_height);//negativity due to window coord is upside-down
            m_vPoints[i].rel_y=scaled_y+float(m_y_pos+m_height);
        }
    }
    else//static y range
    {
        float range=m_window_y_max-m_window_y_min;
        if(range==0) range=1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            float value=m_vPoints[i].y;
            if(value>m_window_y_max) value=m_window_y_max;//clamp to max
            else if(value<m_window_y_min) value=m_window_y_min;//clamp to min

            float rel_y=(value-m_window_y_min)/range;
            float scaled_y=rel_y*float(-m_height);//negativity due to window coord is upside-down
            m_vPoints[i].rel_y=scaled_y+float(m_y_pos+m_height);
        }
    }

    if(m_dyn_x)
    {
        float range=m_max_x-m_min_x;
        if(range==0) range=1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            float rel_x=(m_vPoints[i].x-m_min_x)/range;
            float scaled_x=rel_x*float(m_width);
            m_vPoints[i].rel_x=scaled_x+float(m_x_pos);
        }
    }
    else//static x range (same as dyn x ATM)
    {
        /*float range=m_max_x-m_min_x;
        if(range==0) range=1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            float rel_x=(m_vPoints[i].x-m_min_x)/range;
            float scaled_x=rel_x*float(m_width);
            m_vPoints[i].rel_x=scaled_x+float(m_x_pos);
        }*/

        float range=m_max_x-m_min_x;
        if(range==0) range=1;
        for(int i=0;i<(int)m_vPoints.size();i++)
        {
            float value=m_vPoints[i].x;
            //if(value>m_window_y_max) value=m_window_y_max;//clamp to max
            //else if(value<m_window_y_min) value=m_window_y_min;//clamp to min

            float rel_x=(value-m_min_x)/range;
            float scaled_x=rel_x*float(m_width);
            m_vPoints[i].rel_x=scaled_x+m_x_pos;//+float(m_x_pos+m_width);
        }
    }

    //cout<<m_min_x<<' '<<m_max_x<<" , "<<m_min_y<<' '<<m_max_y<<endl;//temp xxxxxxxx

    return true;
}

bool graph::clean_up(int cut_value)//erase number of points, skip one, erase number of points
{
    if(m_vPoints.empty()||cut_value<0) return false; //should not happen

    if(cut_value==0)//empty vector
    {
        m_vPoints.clear();
        return true;
    }

    //uneven distribution, old values have bigger gap than new values, x test needed
    float range=m_max_x-m_min_x;
    float average_dist=range/(float)m_vPoints.size();
    float last_x=m_vPoints[0].x;

    for(int i=1;i<(int)m_vPoints.size()-cut_value;i++)
    {
        if(m_vPoints[i].x>last_x+average_dist)//erase only if point is separated more than the average
        {
            last_x=m_vPoints[i].x;
        }
        else
        {
            m_vPoints.erase(m_vPoints.begin()+i,m_vPoints.begin()+i+cut_value);
            i--;//next point have the current index after erase, therefor -1 now and +1 after continue
        }
    }

    return true;
}

bool graph::set_view_x(bool dyn_x,int window_size)
{
    m_dyn_x=dyn_x;
    m_window_size_x=window_size;

    return true;
}

bool graph::set_view_y(bool dyn_y,int y_min,int y_max)
{
    m_dyn_y=dyn_y;
    m_window_y_min=y_min;
    m_window_y_max=y_max;

    pre_draw_calc();

    return true;
}

bool graph::set_view_window(float extremes[4])
{
    m_min_x=extremes[0];
    m_max_x=extremes[1];
    m_min_y=extremes[2];
    m_max_y=extremes[3];

    m_dyn_x=false;
    m_dyn_y=false;

    m_window_y_min=m_min_y;
    m_window_y_max=m_max_y;

    pre_draw_calc();

    return true;
}

bool graph::get_view_window(float extremes[4])
{
    extremes[0]=m_min_x;
    extremes[1]=m_max_x;
    extremes[2]=m_min_y;
    extremes[3]=m_max_y;

    return true;
}

bool graph::get_y_extremes(float y_extremes[2],float x_box[2])//min max
{
    float y_min,y_max;
    bool first=true;
    for(int i=0;i<(int)m_vPoints.size();i++)
    {
        if(m_vPoints[i].x>=x_box[0] && m_vPoints[i].x<=x_box[1])
        {
            if(first)
            {
                first=false;
                //init values
                y_min=y_max=m_vPoints[i].y;
            }
            else
            {
                if     (m_vPoints[i].y>y_max) y_max=m_vPoints[i].y;
                else if(m_vPoints[i].y<y_min) y_min=m_vPoints[i].y;
            }
        }
        if(m_vPoints[i].x>x_box[1]) break;
    }

    y_extremes[0]=y_min;
    y_extremes[1]=y_max;

    return true;
}

bool graph::draw_graph(void)
{
    if(!m_ready) return false;

    /*//draw frame....
    glColor3f(1,1,1);
    glBegin(GL_LINE_STRIP);
    glVertex2f(m_x_pos,m_y_pos);
    glVertex2f(m_x_pos,m_y_pos+m_height);
    glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
    glVertex2f(m_x_pos+m_width,m_y_pos);
    glVertex2f(m_x_pos,m_y_pos);
    glEnd();*/

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(m_color[0],m_color[1],m_color[2],m_color[3]);

    switch(m_status)
    {
        case 0://hide
        {
            ;
        }break;

        case 1://more transparent
        {
            //glColor4f(0,0.8,0,0.5);
            glLineWidth(1);
            //glEnable(GL_BLEND);
            glBegin(GL_LINE_STRIP);
            for(int i=0;i<(int)m_vPoints.size();i++)
            {
                glVertex2f(m_vPoints[i].rel_x,m_vPoints[i].rel_y);
            }
            glEnd();
            glDisable(GL_BLEND);
        }break;

        case 2://less transparent
        {
            //glColor4f(0.3,0.9,0.3,0.8);
            glLineWidth(3);
            //glEnable(GL_BLEND);
            glBegin(GL_LINE_STRIP);
            for(int i=0;i<(int)m_vPoints.size();i++)
            {
                glVertex2f(m_vPoints[i].rel_x,m_vPoints[i].rel_y);
            }
            glEnd();
            glLineWidth(1);
            //glDisable(GL_BLEND);
        }break;

        case 3://show
        {
            //glColor3f(0,1,0);
            glLineWidth(3);
            glBegin(GL_LINE_STRIP);
            for(int i=0;i<(int)m_vPoints.size();i++)
            {
                glVertex2f(m_vPoints[i].rel_x,m_vPoints[i].rel_y);
            }
            glEnd();
            glLineWidth(1);
        }break;
    }

    glDisable(GL_BLEND);

    return true;
}
