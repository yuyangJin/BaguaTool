import sys                                                                                                                                                    
import os                                                                                                                                                     
proj_dir = os.environ['BAGUA_DIR']                                                                                                                            
sys.path.append(proj_dir + r"/python")                                                                                                                 
import json
from pag import *   
from graphvizoutput import *                                                                                                                                           
#import ProgramAbstractionGraph as paag  


file_name = sys.argv[1]
output_file_name = sys.argv[2]
if output_file_name == None:
    output_file_name = "output"
g = ProgramAbstractionGraph.Read_GML(file_name)                                                                                                         
graphviz_output = GraphvizOutput(output_file = output_file_name)
graphviz_output.draw(g, vertex_attrs = ["id", "name", "type", "saddr", "eaddr"], edge_attrs = ["id"]) #, vertex_color_depth_attr = "CYCAVGPERCENT")
graphviz_output.show()

#g.write_dot("test.dot")

