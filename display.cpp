#include "display.h"

display::display()
{
    m_ready=false;
}

display::display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],float value)
{
    m_show_border=false;
    m_border_color[0]=0.5; m_border_color[1]=0.5; m_border_color[2]=0.5;

    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_max_char=max_char;
    m_show_value=true;
    m_show_text=false;
    m_value=value;
    m_font_texture[0]=font_texture[0];
    m_font_texture[1]=font_texture[1];
    m_font_texture[2]=font_texture[2];
    m_bright_font=true;
    m_text_align=3;

    //float to string conversion
    ostringstream buffer;
    buffer << value;;
    string text=buffer.str();
    //clamping
    if((int)text.size()>m_max_char)
    {
        text.erase(text.begin()+max_char,text.end());
    }
    m_text=text;

    calc_text_scale();

    m_ready=true;
}

display::display(int x_pos,int y_pos,int width,int height,int max_char,int font_texture[3],string text)
{
    m_show_border=false;
    m_border_color[0]=0.5; m_border_color[1]=0.5; m_border_color[2]=0.5;

    m_x_pos=x_pos;
    m_y_pos=y_pos;
    m_width=width;
    m_height=height;

    m_max_char=max_char;
    m_show_text=true;
    m_show_value=false;
    m_text=text;
    m_font_texture[0]=font_texture[0];
    m_font_texture[1]=font_texture[1];
    m_font_texture[2]=font_texture[2];
    m_bright_font=true;
    m_text_align=3;

    calc_text_scale();

    m_ready=true;
}

bool display::set_pos(float new_pos[2])
{
    m_x_pos=new_pos[0];
    m_y_pos=new_pos[1];

    return true;
}

bool display::get_pos(float curr_pos[2])
{
    curr_pos[0]=m_x_pos;
    curr_pos[1]=m_y_pos;

    return true;
}

bool display::set_value(float value)
{
    m_show_text=false;
    m_show_value=true;

    m_value=value;

    if( m_value>=1000000 ) //round if too big  ex:12345678
    {
        //count numbers
        int numbers=1;
        float temp_value=m_value;//12345678
        for(int i=0;i<m_max_char;i++)
        {
            if( temp_value/10 >= 1 )
            {
                temp_value/=10;
                numbers++;
            }
            else break;//numbers=8
        }
        //get first 6 values, dividing
        float divider=1;
        for(int i=0;i<numbers-6;i++)//2
        {
            divider*=10;//divider=100
        }
        temp_value=int(m_value/divider);//123456.78

        //string conversion
        ostringstream buffer;
        buffer << temp_value;
        string text=buffer.str();
        //add extra zeroes
        for(int i=0;i<numbers-6;i++)//2
        {
            text.append(1,'0');
        }
        m_text=text;
    }
    else
    {
        //string conversion
        ostringstream buffer;
        buffer<<m_value;
        string text=buffer.str();
        m_text=text;

        //check precision
        bool found_dot=false;
        for(int i=0;i<(int)text.size();i++)
        {
            if(text[i]=='.')
            {
                found_dot=true;
                break;
            }
        }
        if(!found_dot)
        {
            //add dot and 2 zeroes
            m_text.append(".00");
        }
    }

    calc_text_scale();

    return true;
}

float display::get_value(void)
{
    return m_value;
}

bool display::add_value(float value)
{
    m_show_text=false;
    m_show_value=true;

    m_value+=value;

    if( m_value>=1000000 ) //round if too big  ex:12345678
    {
        //count numbers
        int numbers=1;
        float temp_value=m_value;//12345678
        for(int i=0;i<m_max_char;i++)
        {
            if( temp_value/10 >= 1 )
            {
                temp_value/=10;
                numbers++;
            }
            else break;//numbers=8
        }
        //get first 6 values, dividing
        float divider=1;
        for(int i=0;i<numbers-6;i++)//2
        {
            divider*=10;//divider=100
        }
        temp_value=int(m_value/divider);//123456.78

        //string conversion
        ostringstream buffer;
        buffer << temp_value;
        string text=buffer.str();
        //add extra zeroes
        for(int i=0;i<numbers-6;i++)//2
        {
            text.append(1,'0');
        }
        m_text=text;

    }
    else
    {
        //string conversion
        ostringstream buffer;
        buffer << m_value;
        string text=buffer.str();
        m_text=text;
    }

    calc_text_scale();

    return true;
}

bool display::set_text(string text)
{
    m_show_text=true;
    m_show_value=false;

    m_text=text;
    calc_text_scale();

    return true;
}

string display::get_text(void)
{
    return m_text;
}

bool display::draw_display(void)
{
    if(!m_ready) return false;

    if(m_show_border) //draw frame....
    {
        glColor3f(m_border_color[0],m_border_color[1],m_border_color[2]);
        glBegin(GL_LINE_STRIP);
        glVertex2f(m_x_pos,m_y_pos);
        glVertex2f(m_x_pos,m_y_pos+m_height);
        glVertex2f(m_x_pos+m_width,m_y_pos+m_height);
        glVertex2f(m_x_pos+m_width,m_y_pos);
        glVertex2f(m_x_pos,m_y_pos);
        glEnd();
    }

    string text=m_text;//symbols to display

    glPushMatrix();
    glColor3f(1,1,1);//allows all colors = white text
    glTranslatef(m_x_pos,m_y_pos+m_height,0);

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, m_font_texture[2]);
    glVertexPointer(3, GL_FLOAT, 0, m_char_plate);

    //draw mask
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR,GL_ZERO);

    switch(m_text_align)
    {
        case 1: draw_letters_left(text); break;
        case 2: draw_letters_right(text); break;
        case 3: draw_letters_center(text); break;
    }

    //draw text
    glDepthMask(GL_TRUE);
    if(m_bright_font) glBindTexture(GL_TEXTURE_2D, m_font_texture[0]);
    else glBindTexture(GL_TEXTURE_2D, m_font_texture[1]);
    glBlendFunc(GL_ONE,GL_ONE);

    switch(m_text_align)
    {
        case 1: draw_letters_left(text); break;
        case 2: draw_letters_right(text); break;
        case 3: draw_letters_center(text); break;
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();

    return true;
}

bool display::calc_text_scale(void)
{
    //clamp size
    if((int)m_text.size()>m_max_char)
    {
        m_text.erase(m_text.begin()+m_max_char,m_text.end());
    }

    //calc size of char_plate
    float height=m_height;
    int char_length=(int)m_text.size();

    if(char_length<1)
    {
        //set standard size
        m_char_plate[0]=0; m_char_plate[1]=-10; m_char_plate[2]=0; //top left
        m_char_plate[3]=0; m_char_plate[4]=0; m_char_plate[5]=0; //bottom left
        m_char_plate[6]=10; m_char_plate[7]=0; m_char_plate[8]=0; //bottom right
        m_char_plate[9]=10; m_char_plate[10]=-10; m_char_plate[11]=0; //top right
        return false; //error
    }

    float width=(float)m_width/(float)char_length;
    //clamp to rational height ratio (0.54:1)
    if(width>0.54*height) width=0.54*height;
    m_char_width=width;

    m_char_plate[0]=0; m_char_plate[1]=-height; m_char_plate[2]=0; //bottom left
    m_char_plate[3]=0; m_char_plate[4]=0; m_char_plate[5]=0; //top left
    m_char_plate[6]=width; m_char_plate[7]=0; m_char_plate[8]=0; //top right
    m_char_plate[9]=width; m_char_plate[10]=-height; m_char_plate[11]=0; //bottom right

    return true;
}

bool display::draw_letters_left(string text)
{
    glPushMatrix();
    for(int letter=0;letter<(int)text.size();letter++)
    {
        float init_off_x=0.0f/1024.0f; float init_off_y=1.0f;
        float tex_width=22.51f/1024.0f; float tex_height=42.0f/1024.0f;
        float tex_x_offset=0.0; float tex_y_offset=1.0;

        int char_id=int(text[letter])-32;// -32 due to start value is 32, but now 0

        //åäö test
        if(text[letter]=='\x86') char_id=95; //pos in font texture å
        if(text[letter]=='\x84') char_id=96; //pos in font texture ä
        if(text[letter]=='\x94') char_id=97; //pos in font texture ö
        if(text[letter]=='\x8F') char_id=98; //pos in font texture Å
        if(text[letter]=='\x8E') char_id=99; //pos in font texture Ä
        if(text[letter]=='\x99') char_id=100;//pos in font texture Ö

        while(char_id>44)//get y offset
        {
            char_id-=45;//reset x offset
            tex_y_offset-=tex_height;
        }
        tex_x_offset=init_off_x+tex_width*(float)char_id;

        float letter_tex[]={ tex_x_offset,           tex_y_offset,
                             tex_x_offset,           tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset };

        glTexCoordPointer(2, GL_FLOAT, 0, letter_tex);
        glDrawArrays(GL_QUADS, 0, 4);
        glTranslatef(m_char_width,0,0);//move to position for next letter
    }
    glPopMatrix();


    return true;
}

bool display::draw_letters_right(string text)
{
    glPushMatrix();
    //move to right side
    glTranslatef( m_width - m_char_width ,0,0  );
    for(int letter=(int)text.size()-1;letter>=0;letter--)
    {
        float init_off_x=0.0f/1024.0f; float init_off_y=1.0f;
        float tex_width=22.51f/1024.0f; float tex_height=42.0f/1024.0f;
        float tex_x_offset=0.0; float tex_y_offset=1.0;

        int char_id=int(text[letter])-32;// -32 due to start value is 32, but now 0

        //åäö test
        if(text[letter]=='\x86') char_id=95; //pos in font texture å
        if(text[letter]=='\x84') char_id=96; //pos in font texture ä
        if(text[letter]=='\x94') char_id=97; //pos in font texture ö
        if(text[letter]=='\x8F') char_id=98; //pos in font texture Å
        if(text[letter]=='\x8E') char_id=99; //pos in font texture Ä
        if(text[letter]=='\x99') char_id=100;//pos in font texture Ö

        while(char_id>44)//get y offset
        {
            char_id-=45;//reset x offset
            tex_y_offset-=tex_height;
        }
        tex_x_offset=init_off_x+tex_width*(float)char_id;

        float letter_tex[]={ tex_x_offset,           tex_y_offset,
                             tex_x_offset,           tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset };

        glTexCoordPointer(2, GL_FLOAT, 0, letter_tex);
        glDrawArrays(GL_QUADS, 0, 4);
        glTranslatef(-m_char_width,0,0);//move to position for next letter
    }
    glPopMatrix();


    return true;
}

bool display::draw_letters_center(string text)
{
    glPushMatrix();
    int letters_width=m_char_width*(float)text.size();
    //move to center
    glTranslatef( (m_width-letters_width)*0.5 ,0,0  );
    for(int letter=0;letter<(int)text.size();letter++)
    {
        float init_off_x=0.0f/1024.0f; float init_off_y=1.0f;
        float tex_width=22.51f/1024.0f; float tex_height=42.0f/1024.0f;
        float tex_x_offset=0.0; float tex_y_offset=1.0;

        int char_id=int(text[letter])-32;// -32 due to start value is 32, but now 0

        //åäö test
        if(text[letter]=='\x86') char_id=95; //pos in font texture å
        if(text[letter]=='\x84') char_id=96; //pos in font texture ä
        if(text[letter]=='\x94') char_id=97; //pos in font texture ö
        if(text[letter]=='\x8F') char_id=98; //pos in font texture Å
        if(text[letter]=='\x8E') char_id=99; //pos in font texture Ä
        if(text[letter]=='\x99') char_id=100;//pos in font texture Ö

        while(char_id>44)//get y offset
        {
            char_id-=45;//reset x offset
            tex_y_offset-=tex_height;
        }
        tex_x_offset=init_off_x+tex_width*(float)char_id;

        float letter_tex[]={ tex_x_offset,           tex_y_offset,
                             tex_x_offset,           tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset-tex_height,
                             tex_x_offset+tex_width, tex_y_offset };

        glTexCoordPointer(2, GL_FLOAT, 0, letter_tex);
        glDrawArrays(GL_QUADS, 0, 4);
        glTranslatef(m_char_width,0,0);//move to position for next letter
    }
    glPopMatrix();


    return true;
}

bool display::setting_flags(bool border)
{
    m_show_border=border;

    return true;
}

bool display::setting_flags(bool border,float colorRGB[3])
{
    m_show_border=border;
    m_border_color[0]=colorRGB[0];
    m_border_color[1]=colorRGB[1];
    m_border_color[2]=colorRGB[2];

    return true;
}
