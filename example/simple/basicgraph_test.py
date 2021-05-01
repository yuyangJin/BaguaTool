import sys                                                                                                                                                    
import os                                                                                                                                                     
proj_dir = os.environ['BAGUA_DIR']                                                                                                                            
sys.path.append(proj_dir + r"/python")                                                                                                                 
import json
from pag import *   
from graphvizoutput import *                                                                                                                                           
#import ProgramAbstractionGraph as paag  

g = ProgramAbstractionGraph.Read_GML("root_3.gml")                                                                                                         
graphviz_output = GraphvizOutput(output_file = "root_single_process")
graphviz_output.draw(g, vertex_attrs = ["id", "name", "type", "saddr", "eaddr", "TOTCYCAVG"], edge_attrs = ["id"])
graphviz_output.show()

#g.write_dot("test.dot")

