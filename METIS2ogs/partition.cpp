// expre_new_Operator.cpp
// compile with: /EHsc
#include <cmath>


#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <time.h>
#include <vector>


#include "Mesh.h"

using namespace std;
using namespace Mesh_Group;

#define ver "V2.0. 2012"

void Version()
{  
  cout<<"\nOpenGeoSys interface to METIS"<<endl;
  cout<<ver<<endl;
  cout<<"Written by wenqing.wang@ufz.de."<<endl<<endl;
}
void OptionList()
{ 
  string s_intro = "The task of this tool is twofold:" 
	             " to convert ogs mesh file into METIS input file for domain decomposition,"
                  " and to use the METIS domain decomposition data to partition"
				  "the ogs finite element meshes for parallel computing.\n"
				  "Note: input mesh file must only contain linear elements";
  cout << s_intro<<endl<<endl;
  cout << "Tasks:\n  --version\n  --help\n  --ogs2metis\n  --metis2ogs\n"<<endl;
  cout << "Option for --metis2ogs task:"<<endl;
  cout << "  -q           generate quadratic elements. It can be ommitted if quadratic element is not used."<<endl;
  cout << "  -np[number]  number of partitions. There must be no space between np and number"<<endl;
  cout << "  -e           partition by element (non overlapped subdomain)"<<endl;
  cout << "  -n           partition by node (overlapped subdomain)"<<endl;
  cout << "  -odom        output subdomain mesh"<<endl;
}

void FindFileNameInCommand(stringstream &ss, string &fname)
{
  while(!ss.eof())
  {
      ss>>fname;
      if(   !(fname.find("-")!=string::npos || fname.find("--")!=string::npos) )
      {
           return; 
      } 
  }
}

enum Task {metis2ogs, ogs2metis};
enum PartType {by_element, by_node};

int main(int argc, char* argv[])
{
  ifstream infile;
  fstream ofile;
  stringstream ss;

  Task this_task;
  PartType part_type = by_node;

  bool quad = false;
  bool out_subdom = false;

  //ios::pos_type position;

  string s_buff;
  string fname;
  string fpath;

  int nparts = 1; 
  string str_nparts;

  Version();
  if(argc>1) 
  {
      for(int i=0; i<argc; i++)
	  {
          s_buff = argv[i];
          if(s_buff.find("-e")!=string::npos)
             part_type = by_element;
          else if(s_buff.find("-n")!=string::npos)
             part_type = by_node;

          if(s_buff.find("-q")!=string::npos) 
             quad = true; 
   
          if(s_buff.find("-odom")!=string::npos)
          { 
             out_subdom = true; 
          }
		   
		  // Number of partitions
		  if(s_buff.find("-np")!=string::npos) 
		  {
             size_t pos;
			 pos = s_buff.find_first_of("p");
			 s_buff = s_buff.substr(pos+1);
			 nparts = atoi(s_buff.c_str());
			 str_nparts = s_buff;
		  }    

          if(s_buff.find("ogs2metis")!=string::npos)
          { 
             this_task = ogs2metis;
		  }
          else if(s_buff.find("metis2ogs")!=string::npos)
          { 
             this_task = metis2ogs;
		  }
          else if(s_buff.find("help")!=string::npos)
          { 
             //Version();
             OptionList();
			 exit(0);
		  }
          else if(s_buff.find("version")!=string::npos)
          { 
             cout<<ver;
             exit(0);
		  }


          if(   !(s_buff.find("-")!=string::npos || s_buff.find("--")!=string::npos) )
		  {
              fname = s_buff;
		  }
                             
	  }
  }
  else //terminal
  {
    OptionList();
    cout<<"\nInput task, options and file name (non extension):\n ";
   
	getline(cin, s_buff);
	ss.str(s_buff);
    if(s_buff.find("ogs2metis")!=string::npos)
	{ 
       this_task = ogs2metis; 
       if(s_buff.find("-e")!=string::npos || s_buff.find("-n")!=string::npos||s_buff.find("-q")!=string::npos)
	   { 
          cout<<"Warning: option is not needed for this task"<<endl; 
       }  

       FindFileNameInCommand(ss, fname);
	}   
    else if(s_buff.find("metis2ogs")!=string::npos)
	{ 
       this_task = metis2ogs; 
       if(s_buff.find("-e")!=string::npos)
          part_type = by_element;
       else if(s_buff.find("-n")!=string::npos)
          part_type = by_node;

       if(s_buff.find("-q")!=string::npos)
	   { 
          quad = true; 
       }
       if(s_buff.find("-odom")!=string::npos)
	   { 
          out_subdom = true; 
       }

	   // Find name
       FindFileNameInCommand(ss, fname);
	   ss.clear();

	   // Find partition number
       ss.str(s_buff);
       while(!ss.eof())
       {
           ss>>s_buff;
           if(s_buff.find("-np")!=string::npos)
		   {
              size_t pos;
			  pos = s_buff.find_first_of("p");
              s_buff = s_buff.substr(pos+1);
              nparts = atoi(s_buff.c_str());
              str_nparts = s_buff;
              break;
		   }

       }

	}   
	ss.clear();
    
  }
    

   //Get the path to the folder where the input file is. 
   size_t pos_end;
   pos_end = fname.find_last_of('\\');
   //
   if(pos_end != std::string::npos)
   {
       fpath = fname.substr(0,pos_end) + "\\";
   }
   else 
   {
       pos_end = fname.find_last_of('/');
       if(pos_end != std::string::npos) 
          fpath = fname.substr(0,pos_end) + "/";
   }


  infile.open(fname+".msh");
  if(!infile.is_open())
  {
      cerr<<("Error: cannot open msh file . It may not exist !");
      exit(1);
  }


  clock_t elp_time;
  elp_time = -clock();

  Mesh_Group::Mesh *a_mesh = new Mesh();

  bool rfiMesh = true;
  string line_string;
  getline(infile,line_string); // The first line
  if(line_string.find("#FEM_MSH")!=string::npos)
    rfiMesh = false;
  if(line_string.find("GeoSys-MSH")!=string::npos) 
    rfiMesh = false;
  infile.seekg(0L,ios::beg);

  if(rfiMesh)
     a_mesh->ReadGrid(infile);
  else
     a_mesh->ReadGridGeoSys(infile);

  
  switch(this_task)
  {
     case ogs2metis:
        ofile.open(fname+".mesh", ios::out );         
        a_mesh->Write2METIS(ofile);

        break;
	 case metis2ogs:
        cout<<"\n***Compute mesh topology"<<endl;
        a_mesh->ConstructGrid();  

		s_buff = fpath+"partdmesh "  + fname + ".mesh " + str_nparts;

		if(!system(s_buff.c_str()))
		{
           cout<<"METIS executable file may not be found "<<endl;
           exit(1);
		} 

        cout<<"\n***Prepare subdomain mesh"<<endl;
        if(part_type == by_element)
            a_mesh->ConstructSubDomain_by_Elements(fname.c_str(), nparts, out_subdom);
		else if(part_type == by_node)
		{
           if(quad)
	          a_mesh->GenerateHighOrderNodes();
           
		   a_mesh->ConstructSubDomain_by_Nodes(fname.c_str(), nparts, quad, out_subdom); 
		}
        break;
	 default:
        break;
  }

  delete a_mesh;

  elp_time += clock();
  cout<<"\n***Total CPU time elapsed: "
      <<(double)elp_time / CLOCKS_PER_SEC<<"s"<<endl;
 
  return 0;

}
 