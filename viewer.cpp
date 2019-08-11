#include "viewer.h"

viewer::viewer()
{
    //ctor
}

bool viewer::init(int screen_size[2],bool keys[256],int mouse_pos[2],bool mouse_but[2],string input_path,string run_path)
{
    pMouse_pos=mouse_pos;
    pMouse_but=mouse_but;
    pKeys=keys;
    m_draw_mode=m_zoom_mode=m_just_deleted=m_delete_possible=false;
    m_screen_size[0]=screen_size[0];
    m_screen_size[1]=screen_size[1];

    //load data default from textfile INPUT_DATA.txt
    ifstream ifs_input("INPUT_DATA.txt");
    if(input_path.empty() || input_path=="debug")
    {
        if(ifs_input==0)
        {
            cout<<"ERROR: Could not load input file\n";
            //try to create example file
            ofstream ofs_temp("INPUT_DATA.txt");
            if(!ofs_temp==0)
            {
                ofs_temp<<"TITLE\nEXTRA LINE\n123.456\t0.5\n125.456\t0.9\n129.456\t0.3\n";
                ofs_temp.close();
            }
            return false;
        }
    }
    else//use path
    {
        ifs_input.close();
        //erase '"'
        for(int i=0;i<(int)input_path.size();i++)
        {
            if(input_path[i]=='\"')
            {
                input_path.erase(input_path.begin()+i);
                i--;
            }
        }
        ifs_input.open(input_path.c_str());
        if(ifs_input==0)
        {
            cout<<"ERROR: Could not load input file: ";
            cout<<input_path<<endl;
            return false;
        }
    }


    float x_max=-1;
    float x_min=-1;
    float y_max=-1;
    float y_min=-1;
    string line,word;
    getline(ifs_input,line);//skip
    getline(ifs_input,line);//skip
    while( getline(ifs_input,line) )
    {
        stringstream ss(line);
        ss>>word;
        float x_val=atof( word.c_str() );
        ss>>word;
        float y_val=atof( word.c_str() );

        //cout<<x_val<<", "<<y_val<<endl;

        //get extremes
        if(m_vec_data.empty())//init values
        {
            x_max=x_min=x_val;
            y_max=y_min=y_val;
        }
        else
        {
            if     (x_val>x_max) x_max=x_val;
            else if(x_val<x_min) x_min=x_val;

            if     (y_val>y_max) y_max=y_val;
            else if(y_val<y_min) y_min=y_val;
        }

        //store
        m_vec_data.push_back( st_data_point(x_val,y_val) );
    }
    ifs_input.close();

    //calc pix2Da factor
    m_pix2Da=(x_max-x_min)/(float)screen_size[0];
    m_pix2y_scale=(y_max-y_min)/(float)screen_size[1];

    //init graph
    m_graph=graph(0,0,screen_size[0],screen_size[1],true,true);
    for(int i=0;i<(int)m_vec_data.size();i++)
    {
        m_graph.add_point_without_calc( m_vec_data[i].x,m_vec_data[i].y );
    }
    m_graph.force_pre_draw_calc();

    //load text textures
    while((int)run_path.size()>0)
    {
        if(run_path[(int)run_path.size()-1]=='\\') break;
        else run_path.erase(run_path.begin()+(int)run_path.size()-1);
    }
    string path1=run_path;
    path1.append("data\\default_light.bmp");
    string path2=run_path;
    path2.append("data\\default_dark.bmp");
    string path3=run_path;
    path3.append("data\\default_mask.bmp");

    m_text_texture[0]=SOIL_load_OGL_texture
	(
		path1.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);
	m_text_texture[1]=SOIL_load_OGL_texture
	(
		path2.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);
	m_text_texture[2]=SOIL_load_OGL_texture
	(
		path3.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(m_text_texture[0]==0 || m_text_texture[1]==0 || m_text_texture[2]==0)
    {
        cout<<"ERROR: Could not load font textures\n";
        return false;
    }

    return true;
}

bool viewer::cycle(void)
{
    update();
    draw();

    return true;
}

bool viewer::update(void)
{
    bool key_trigger_LMB=false;//on if clicked this cycle, can only be used once

    //toggle drawing mode (RIGHT MB)
    if(!m_draw_mode && pMouse_but[1])
    {
        m_draw_mode=true;
        //enable drawing
        m_draw_xpos_start=m_draw_xpos_curr=pMouse_pos[0];
        m_draw_ypos_start=pMouse_pos[1];
        //create new display box
        m_vec_displays.push_back( display(0,0,_disp_width,_disp_height,6,m_text_texture,0) );
        m_vec_displays_aa.push_back( display(0,0,_disp_width_text,_disp_height,20,m_text_texture," ") );
    }
    if(m_draw_mode && !pMouse_but[1])
    {
        m_draw_mode=false;
        //store drawed line
        if(m_draw_xpos_start!=m_draw_xpos_curr)
        {
            m_vec_lines.push_back( st_line_data(m_draw_xpos_start,
                                                m_draw_xpos_curr,
                                                m_draw_ypos_start) );

        }
        else//remove displaybox with zero
        {
            m_vec_displays.pop_back();
            m_vec_displays_aa.pop_back();
        }
    }

    //toggle zoom mode (LEFT MB)
    if(!m_zoom_mode && pMouse_but[0])
    {
        m_zoom_mode=true;
        //set start values
        m_zoom_pos_start[0]=m_zoom_pos_end[0]=pMouse_pos[0];
        m_zoom_pos_start[1]=m_zoom_pos_end[1]=pMouse_pos[1];

        key_trigger_LMB=true;//will enable mass display to be placed this cycle
        m_delete_possible=false;
    }
    if(m_zoom_mode && !pMouse_but[0])
    {
        m_zoom_mode=false;
        m_delete_possible=true;

        //avoid min zoom
        if( (m_zoom_pos_end[0]>m_zoom_pos_start[0]+_zoom_pix_min ||
             m_zoom_pos_end[0]<m_zoom_pos_start[0]-_zoom_pix_min) &&
            (m_zoom_pos_end[1]>m_zoom_pos_start[1]+_zoom_pix_min ||
             m_zoom_pos_end[1]<m_zoom_pos_start[1]-_zoom_pix_min) )
        {
            //go to selected zoom
            float window_limits_old[4];
            float window_limits[4];
            m_graph.get_view_window(window_limits_old);
            if(m_zoom_pos_start[0]<m_zoom_pos_end[0])
            {
                window_limits[1]=m_zoom_pos_end[0]*m_pix2Da+window_limits_old[0];//xmin
                window_limits[0]=m_zoom_pos_start[0]*m_pix2Da+window_limits_old[0];//xmax
            }
            else//flipped box
            {
                window_limits[1]=m_zoom_pos_start[0]*m_pix2Da+window_limits_old[0];//xmin
                window_limits[0]=m_zoom_pos_end[0]*m_pix2Da+window_limits_old[0];//xmax
            }
            if(m_zoom_pos_start[1]<m_zoom_pos_end[1])
            {
                window_limits[3]=(m_screen_size[1]-m_zoom_pos_start[1])*m_pix2y_scale+window_limits_old[2];//ymin
                window_limits[2]=(m_screen_size[1]-m_zoom_pos_end[1])*m_pix2y_scale+window_limits_old[2];//ymax
            }
            else
            {
                window_limits[3]=(m_screen_size[1]-m_zoom_pos_end[1])*m_pix2y_scale+window_limits_old[2];//ymin
                window_limits[2]=(m_screen_size[1]-m_zoom_pos_start[1])*m_pix2y_scale+window_limits_old[2];//ymax
            }
            //ignore y zoom
            //window_limits[2]=window_limits_old[2];
            //window_limits[3]=window_limits_old[3];

            /*cout<<"OLD: ";
            for(int i=0;i<4;i++) cout<<window_limits_old[i]<<", ";
            cout<<"\nNEW: ";
            for(int i=0;i<4;i++) cout<<window_limits[i]<<", ";
            cout<<endl;*/

            //autoscale y
            float y_extremes[2];
            float x_box[2]={window_limits[0],window_limits[1]};
            m_graph.get_y_extremes(y_extremes,x_box);
            window_limits[2]=y_extremes[0];
            window_limits[3]=y_extremes[1];
            //cout<<"X start: "<<window_limits[0]<<"\tX end: "<<window_limits[1]<<endl;
            //cout<<"Min y: "<<y_extremes[0]<<"\tMax y: "<<y_extremes[1]<<endl;

            m_graph.set_view_window(window_limits);

            //clear lines/displays
            m_vec_displays.clear();
            m_vec_displays_aa.clear();
            m_vec_displays_spec_mass.clear();
            m_vec_lines.clear();

            //update scales
            m_pix2Da=(window_limits[1]-window_limits[0])/(float)m_screen_size[0];
            m_pix2y_scale=(window_limits[3]-window_limits[2])/(float)m_screen_size[1];
        }
    }

    //update current displaybox pos
    if(m_draw_mode)
    {
        m_draw_xpos_curr=pMouse_pos[0];
        //move curent display to middle of the line
        float new_pos[2]={m_draw_xpos_start+(m_draw_xpos_curr-m_draw_xpos_start)*0.5-_disp_width*0.5,
                          m_draw_ypos_start-_disp_height};
        m_vec_displays.back().set_pos(new_pos);
        float new_pos_aa[2]={m_draw_xpos_start+(m_draw_xpos_curr-m_draw_xpos_start)*0.5-_disp_width_text*0.5,
                             m_draw_ypos_start};
        m_vec_displays_aa.back().set_pos(new_pos_aa);
        //update value
        float length=pMouse_pos[0]-m_draw_xpos_start;
        m_vec_displays.back().set_value( fabs(length*m_pix2Da) );
        //test if value match known aa
        string aa_string;
        aa_string=get_aa_by_mass( fabs(length*m_pix2Da) );
        m_vec_displays_aa.back().set_text(aa_string);
    }

    //update zoom box
    if(m_zoom_mode && !key_trigger_LMB)
    {
        //set curr values
        m_zoom_pos_end[0]=pMouse_pos[0];
        m_zoom_pos_end[1]=pMouse_pos[1];

        //test if label is selected, move is not allowed
        bool space_taken=false;
        for(int disp_i=0;disp_i<(int)m_vec_displays.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                space_taken=true;
                break;
            }
        }
        //test if inside mass displays
        for(int disp_i=0;disp_i<(int)m_vec_displays_spec_mass.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays_spec_mass[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                space_taken=true;
                break;
            }
        }

        //update mass display pos/value
        if(!m_vec_displays_spec_mass.empty() && !space_taken)
        {
            float new_pos[2]={pMouse_pos[0]-_disp_width*0.5,
                              pMouse_pos[1]-_disp_height};
            m_vec_displays_spec_mass.back().set_pos(new_pos);
            float graph_extremes[4];
            m_graph.get_view_window(graph_extremes);
            float mass_to_display=pMouse_pos[0]*m_pix2Da+graph_extremes[0];
            m_vec_displays_spec_mass.back().set_value(mass_to_display);
        }
    }

    //click to delete line/displaybox
    if(m_delete_possible && !pMouse_but[1])
    {
        //test if inside displaybox
        for(int disp_i=0;disp_i<(int)m_vec_displays.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                m_just_deleted=true;
                //delete this box/line
                m_vec_displays.erase(m_vec_displays.begin()+disp_i);
                m_vec_displays_aa.erase(m_vec_displays_aa.begin()+disp_i);

                m_vec_lines.erase(m_vec_lines.begin()+disp_i);

                break;//only one per cycle
            }
        }
        //test if inside mass displays
        for(int disp_i=0;disp_i<(int)m_vec_displays_spec_mass.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays_spec_mass[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                m_just_deleted=true;
                //delete this box/line
                m_vec_displays_spec_mass.erase(m_vec_displays_spec_mass.begin()+disp_i);

                break;//only one per cycle
            }
        }
    }

    //place new label
    if(pMouse_but[0] && !pMouse_but[1])
    {
        //not if delete is possible at this location
        bool space_taken=false;
        //test if inside displaybox
        for(int disp_i=0;disp_i<(int)m_vec_displays.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                space_taken=true;
                break;
            }
        }
        //test if inside mass displays
        for(int disp_i=0;disp_i<(int)m_vec_displays_spec_mass.size();disp_i++)
        {
            float disp_pos[2];
            m_vec_displays_spec_mass[disp_i].get_pos(disp_pos);
            if(pMouse_pos[0]>disp_pos[0] && pMouse_pos[0]<disp_pos[0]+_disp_width &&
               pMouse_pos[1]>disp_pos[1] && pMouse_pos[1]<disp_pos[1]+_disp_height)
            {
                space_taken=true;
                break;
            }
        }

        //if not deleted, place new display with x-value
        if(key_trigger_LMB && !space_taken)
        {
            key_trigger_LMB=false;

            float disp_pos[2]={pMouse_pos[0]-_disp_width*0.5,
                               pMouse_pos[1]-_disp_height};
            float graph_extremes[4];
            m_graph.get_view_window(graph_extremes);
            float mass_to_display=pMouse_pos[0]*m_pix2Da+graph_extremes[0];
            m_vec_displays_spec_mass.push_back( display(disp_pos[0],disp_pos[1],
                                                        _disp_width,_disp_height,
                                                        6,m_text_texture,mass_to_display) );
        }
    }

    //reset zoom (SPACE 32)
    if(pKeys[32])
    {
        pKeys[32]=false;//manual key reset

        m_graph.set_view_x(true);
        m_graph.set_view_y(true);
        m_graph.force_pre_draw_calc();

        //clear lines/displays
        m_vec_displays.clear();
        m_vec_displays_aa.clear();
        m_vec_displays_spec_mass.clear();
        m_vec_lines.clear();

        //update scales
        float window_limits[4];
        m_graph.get_view_window(window_limits);
        m_pix2Da=(window_limits[1]-window_limits[0])/(float)m_screen_size[0];
        m_pix2y_scale=(window_limits[3]-window_limits[2])/(float)m_screen_size[1];
    }

    m_delete_possible=false;//only valid for one cycle

    return true;
}

bool viewer::draw(void)
{
    //draw spectrum
    m_graph.draw_graph();

    //draw lines
    for(int line_i=0;line_i<(int)m_vec_lines.size();line_i++)
    {
        glColor3f(1,1,1);
        glBegin(GL_LINES);
        glVertex2f(m_vec_lines[line_i].pos_x_start,m_vec_lines[line_i].pos_y);
        glVertex2f(m_vec_lines[line_i].pos_x_end,m_vec_lines[line_i].pos_y);
        glEnd();
    }
    //also current line, if in drawing mode
    if(m_draw_mode)
    {
        glColor3f(1,1,1);
        glBegin(GL_LINES);
        glVertex2f(m_draw_xpos_start,m_draw_ypos_start);
        glVertex2f(m_draw_xpos_curr,m_draw_ypos_start);
        glEnd();
    }

    //draw text boxes
    for(int disp_i=0;disp_i<(int)m_vec_displays.size();disp_i++)
    {
        m_vec_displays[disp_i].draw_display();
        m_vec_displays_aa[disp_i].draw_display();
    }

    for(int disp_i=0;disp_i<(int)m_vec_displays_spec_mass.size();disp_i++)
    {
        m_vec_displays_spec_mass[disp_i].draw_display();
    }

    //draw zoom box
    if(m_zoom_mode)
    {
        glColor3f(0.6,0.9,0.6);
        glBegin(GL_LINE_STRIP);
        glVertex2f(m_zoom_pos_start[0],m_zoom_pos_start[1]);
        glVertex2f(m_zoom_pos_end[0],m_zoom_pos_start[1]);
        glVertex2f(m_zoom_pos_end[0],m_zoom_pos_end[1]);
        glVertex2f(m_zoom_pos_start[0],m_zoom_pos_end[1]);
        glVertex2f(m_zoom_pos_start[0],m_zoom_pos_start[1]);
        glEnd();
    }

    return true;
}

string viewer::get_aa_by_mass(float mass)
{
    string ret_string("");

    float mass_tol=0.5;

    if(mass<57.02146+mass_tol && mass>57.02146-mass_tol)
    {
        ret_string="G";
    }
    if(mass<71.03711+mass_tol && mass>71.03711-mass_tol)
    {
        ret_string="A";
    }
    if(mass<87.03203+mass_tol && mass>87.03203-mass_tol)
    {
        ret_string="S";
    }
    if(mass<97.05276+mass_tol && mass>97.05276-mass_tol)
    {
        ret_string="P";
    }
    if(mass<99.06841+mass_tol && mass>99.06841-mass_tol)
    {
        ret_string="V";
    }
    if(mass<101.04768+mass_tol && mass>101.04768-mass_tol)
    {
        ret_string="T";
    }
    if(mass<103.00919+mass_tol && mass>103.00919-mass_tol)
    {
        ret_string="C";
    }
    if(mass<113.08406+mass_tol && mass>113.08406-mass_tol)
    {
        ret_string="L/I";
    }
    if(mass<114.04293+mass_tol && mass>114.04293-mass_tol)
    {
        ret_string="N";
    }
    if(mass<115.02694+mass_tol && mass>115.02694-mass_tol)
    {
        ret_string="D";
    }
    if(mass<128.05858+mass_tol && mass>128.05858-mass_tol)//mass of Q
    {
        ret_string="Q/K";
    }
    if(mass<129.04259+mass_tol && mass>129.04259-mass_tol)
    {
        ret_string="E";
    }
    if(mass<131.04049+mass_tol && mass>131.04049-mass_tol)
    {
        ret_string="M";
    }
    if(mass<137.05891+mass_tol && mass>137.05891-mass_tol)
    {
        ret_string="H";
    }
    if(mass<147.06841+mass_tol && mass>147.06841-mass_tol)
    {
        ret_string="F";
    }
    if(mass<156.10111+mass_tol && mass>156.10111-mass_tol)
    {
        ret_string="R";
    }
    if(mass<163.06333+mass_tol && mass>163.06333-mass_tol)
    {
        ret_string="Y";
    }
    if(mass<186.07931+mass_tol && mass>186.07931-mass_tol)
    {
        ret_string="W";
    }
    if(mass<159.05059+mass_tol && mass>159.05059-mass_tol)
    {
        ret_string="fMet";
    }

    //guess double aa
    vector<string> vec_matches;
    if(mass>110.0)
    {
        if(!ret_string.empty())
        {
            vec_matches.push_back(ret_string);
            vec_matches.back().append(1,' ');
        }

        for(int i1=0;i1<21;i1++)
        {
            for(int i2=0;i2<21;i2++)
            {
                //never 2 fMet
                if(i1==18 && i2==18) continue;

                if( mass< get_aa_mono_mass(get_aa_by_int(i1))+
                          get_aa_mono_mass(get_aa_by_int(i2))+mass_tol &&
                    mass> get_aa_mono_mass(get_aa_by_int(i1))+
                          get_aa_mono_mass(get_aa_by_int(i2))-mass_tol )
                {
                    //test if new (inverted order)
                    bool is_new=true;
                    for(int match_i=0;match_i<(int)vec_matches.size();match_i++)
                    {
                        if(vec_matches[match_i][0]==get_aa_by_int(i2) &&
                           vec_matches[match_i][1]==get_aa_by_int(i1) )
                        {
                            is_new=false;
                            break;
                        }
                    }
                    if(is_new)
                    {
                        string match(1,get_aa_by_int(i1));
                        //match.append(1,'+');
                        match.append(1,get_aa_by_int(i2));
                        vec_matches.push_back( match );
                    }
                }
            }
        }
    }

    //merge matches if many
    if((int)vec_matches.size()>1)
    {
        string merged;
        for(int i=0;i<(int)vec_matches.size();i++)
        {
            merged.append(vec_matches[i]);
            if(i!=(int)vec_matches.size()-1)
             merged.append(1,',');
        }
        ret_string=merged;
    }
    else if((int)vec_matches.size()==1) ret_string=vec_matches.back();


    /*//guess multi combinations....
    float temp_mass=mass+mass_tol;
    vector<string> vec_alternatives;
    if(!ret_string.empty()) vec_alternatives.push_back(ret_string);

    int numof_aa_max=0;//only G
    int numof_aa_min=1;//only W
    while(temp_mass>0.0)
    {
        numof_aa_min++;
        temp_mass-=get_aa_mono_mass('W');
    }
    temp_mass=mass+mass_tol;
    while(temp_mass>0.0)
    {
        numof_aa_max++;
        temp_mass-=get_aa_mono_mass('G');
    }
    temp_mass=mass+mass_tol;

    //create all combinations......
    vector<st_peptide> vec_peptides;
    //start with single aa peptides
    if(temp_mass>get_aa_mono_mass('G'))
     vec_peptides.push_back( st_peptide('G',get_aa_mono_mass('G')) );
    if(temp_mass>get_aa_mono_mass('A'))
     vec_peptides.push_back( st_peptide('A',get_aa_mono_mass('A')) );
    if(temp_mass>get_aa_mono_mass('S'))
     vec_peptides.push_back( st_peptide('S',get_aa_mono_mass('S')) );
    if(temp_mass>get_aa_mono_mass('P'))
     vec_peptides.push_back( st_peptide('P',get_aa_mono_mass('P')) );
    if(temp_mass>get_aa_mono_mass('V'))
     vec_peptides.push_back( st_peptide('V',get_aa_mono_mass('V')) );
    if(temp_mass>get_aa_mono_mass('T'))
     vec_peptides.push_back( st_peptide('T',get_aa_mono_mass('T')) );
    if(temp_mass>get_aa_mono_mass('C'))
     vec_peptides.push_back( st_peptide('C',get_aa_mono_mass('C')) );
    if(temp_mass>get_aa_mono_mass('L'))
     vec_peptides.push_back( st_peptide('L',get_aa_mono_mass('L')) );
    if(temp_mass>get_aa_mono_mass('I'))
     vec_peptides.push_back( st_peptide('I',get_aa_mono_mass('I')) );
    if(temp_mass>get_aa_mono_mass('N'))
     vec_peptides.push_back( st_peptide('N',get_aa_mono_mass('N')) );
    if(temp_mass>get_aa_mono_mass('D'))
     vec_peptides.push_back( st_peptide('D',get_aa_mono_mass('D')) );
    if(temp_mass>get_aa_mono_mass('Q'))
     vec_peptides.push_back( st_peptide('Q',get_aa_mono_mass('Q')) );
    if(temp_mass>get_aa_mono_mass('K'))
     vec_peptides.push_back( st_peptide('K',get_aa_mono_mass('K')) );
    if(temp_mass>get_aa_mono_mass('E'))
     vec_peptides.push_back( st_peptide('E',get_aa_mono_mass('E')) );
    if(temp_mass>get_aa_mono_mass('M'))
     vec_peptides.push_back( st_peptide('M',get_aa_mono_mass('M')) );
    if(temp_mass>get_aa_mono_mass('H'))
     vec_peptides.push_back( st_peptide('H',get_aa_mono_mass('H')) );
    if(temp_mass>get_aa_mono_mass('F'))
     vec_peptides.push_back( st_peptide('F',get_aa_mono_mass('F')) );
    if(temp_mass>get_aa_mono_mass('R'))
     vec_peptides.push_back( st_peptide('R',get_aa_mono_mass('R')) );
    if(temp_mass>get_aa_mono_mass('Z'))
     vec_peptides.push_back( st_peptide('Z',get_aa_mono_mass('Z')) );
    if(temp_mass>get_aa_mono_mass('Y'))
     vec_peptides.push_back( st_peptide('Y',get_aa_mono_mass('Y')) );
    if(temp_mass>get_aa_mono_mass('W'))
     vec_peptides.push_back( st_peptide('W',get_aa_mono_mass('W')) );


    vector<st_peptide> vec_old_pep;
    vector<st_peptide> vec_new_pep;
    vec_old_pep=vec_peptides;
    for(int numof_aa=1;numof_aa<numof_aa_max;numof_aa++)
    {
        for(int pep=0;pep<(int)vec_old_pep.size();pep++)
        {
            if(temp_mass>vec_old_pep[pep].mass+get_aa_mono_mass(''))
            {
                vec_new_pep.push_back( vec_old_pep[pep]);
                vec_new_pep.back().sequence.append(1,'');
            }
             vec_old_pep[].sequence.append(1,'');
        }
    }*/

    //remove peptides outside error tol


    return ret_string;
}

float viewer::get_aa_mono_mass(char aa)
{
    switch(aa)
    {
        case 'A': return 71.03711;
        case 'R': return 156.10111;
        case 'N': return 114.04293;
        case 'D': return 115.02694;
        case 'C': return 103.00919;
        case 'E': return 129.04259;
        case 'Q': return 128.05858;
        case 'G': return 57.02146;
        case 'H': return 137.05891;
        case 'I': return 113.08406;
        case 'L': return 113.08406;
        case 'K': return 128.09496;
        case 'M': return 131.04049;
        case 'F': return 147.06841;
        case 'P': return 97.05276;
        case 'S': return 87.03203;
        case 'T': return 101.04768;
        case 'W': return 186.07931;
        case 'Y': return 163.06333;
        case 'V': return 99.06841;
        case 'Z': return 159.05059; //fMET
        default:  return 0.0;//error
    }
    return 0.0;//error
}

char viewer::get_aa_by_int(int index)
{
    switch(index)
    {
        case 0: return 'G';
        case 1: return 'A';
        case 2: return 'S';
        case 3: return 'P';
        case 4: return 'V';
        case 5: return 'T';
        case 6: return 'C';
        case 7: return 'L';
        case 8: return 'I';
        case 9: return 'N';
        case 10: return 'D';
        case 11: return 'Q';
        case 12: return 'K';
        case 13: return 'E';
        case 14: return 'M';
        case 15: return 'H';
        case 16: return 'F';
        case 17: return 'R';
        case 18: return 'Z';
        case 19: return 'Y';
        case 20: return 'W';
    }

    return 'x';//error
}
