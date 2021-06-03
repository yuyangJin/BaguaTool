from __future__ import division

import tempfile
import os
import textwrap
import subprocess as sub
import math

from exceptions import PyCallGraphException
from color import Color
from output import Output

class GraphvizOutput(Output):
    def __init__(self, output_file = "", **kwargs):
        self.tool = 'dot'
        #self.tool = 'circo'
        #self.tool = 'neato'
        if output_file == "":
            self.output_files = [self.tool + '.png', self.tool + '.pdf']
            self.output_types = ['png', 'pdf']
        else:
            self.output_files = [output_file + '.png', output_file + '.pdf']
            self.output_types = ['png', 'pdf']
        self.font_name = 'Verdana'
        self.font_size = 7
        self.group_font_size = 10
        self.group_border_color = Color(0, 0, 0, 0.8)
        self.edges = []
        self.vertices = []
        self.preserve_vertices = []
        self.groups = []

        Output.__init__(self, **kwargs)

        self.prepare_graph_attributes()

    @classmethod
    def add_arguments(cls, subparsers, parent_parser, usage):
        defaults = cls()

        subparser = subparsers.add_parser(
            'graphviz', help='Graphviz generation',
            parents=[parent_parser], usage=usage,
        )

        subparser.add_argument(
            '-l', '--tool', dest='tool', default=defaults.tool,
            help='The tool from Graphviz to use, e.g. dot, neato, etc.',
        )

        cls.add_output_file(
            subparser, defaults, 'The generated Graphviz file'
        )

        subparser.add_argument(
            '-f', '--output-format', type=str, default=defaults.output_types[0],
            dest='output_type',
            help='Image format to produce, e.g. png, ps, dot, etc. '
            'See http://www.graphviz.org/doc/info/output.html for more.',
        )

        subparser.add_argument(
            '--font-name', type=str, default=defaults.font_name,
            help='Name of the font to be used',
        )

        subparser.add_argument(
            '--font-size', type=int, default=defaults.font_size,
            help='Size of the font to be used',
        )
    
    def prepare_graph_attributes(self):
        generated_message = '\\n'.join([
            r'Generated by BaguaTool from PACMAN',
        ])

        self.graph_attributes = {
            'graph': {
                'overlap': 'scalexy',
                'fontname': self.font_name,
                'fontsize': self.font_size,
                'fontcolor': Color(0, 0, 0, 0.5).rgba_web(),
                'label': generated_message,
            },
            'node': {
                'fontname': self.font_name,
                'fontsize': self.font_size,
                'fontcolor': Color(0, 0, 0).rgba_web(),
                'style': 'filled',
                'shape': 'rect',
            },
            'edge': {
                'fontname': self.font_name,
                'fontsize': self.font_size,
                'fontcolor': Color(0, 0, 0).rgba_web(),
            }
        }

    def show(self):
        source = self.generate()

        #self.debug(source)

        fd, temp_name = tempfile.mkstemp()
        with os.fdopen(fd, 'w') as f:
            f.write(source)

        for i in range(len(self.output_types)):
            output_type = self.output_types[i]
            output_file = self.output_files[i]
            cmd = '"{0}" -T {1} -o {2} {3}'.format(
                self.tool, output_type, output_file, temp_name
            )

            self.verbose('Executing: {0}'.format(cmd))
            proc = sub.Popen(cmd, stdout=sub.PIPE, stderr=sub.PIPE, shell=True)
            ret, output = proc.communicate()
            if ret:
                os.unlink(temp_name)
                raise PyCallGraphException(
                    'The command "%(cmd)s" failed with error '
                    'code %(ret)i.' % locals())
        #    finally:
        os.unlink(temp_name)

        #self.verbose('Generated {0} with {1} vertices.'.format(
        #    self.output_file, len(self.processor.func_count),
        #))
    
    def draw(self, graph, vertex_attrs = [], edge_attrs = [], vertex_color_depth_attr = "", preserve_attrs = ""):
        ''' Add vertices and edges into self.vertices and self.edges
        '''

        self.vertices = self.generate_vertices(graph.vs, vertex_attrs, vertex_color_depth_attr, preserve_attrs)

        self.edges = self.generate_edges(graph.es, edge_attrs, preserve_attrs)


    def generate(self):
        '''Returns a string with the contents of a DOT file for Graphviz to
        parse.
        '''
        indent_join = '\n' + ' ' * 12

        return textwrap.dedent('''\
        digraph G {{
            // Attributes
            {0}
            // Groups
            {1}
            // Nodes
            {2}
            // Edges
            {3}
        }}
        '''.format(
            indent_join.join(self.generate_attributes()),
            # indent_join.join(self.generate_groups()),
            # indent_join.join(self.generate_vertices()),
            # indent_join.join(self.generate_edges()),
            indent_join.join(self.groups),
            indent_join.join(self.vertices),
            indent_join.join(self.edges),
        ))

    def attrs_from_dict(self, d):
        output = []
        for attr, val in d.items():
            output.append('%s = "%s"' % (attr, val))
        return ', '.join(output)

    def vertex(self, key, attr):
        return '"{0}" [{1}];'.format(
            key, self.attrs_from_dict(attr),
        )

    def edge(self, edge_src, edge_dest, attr):
        return '"{0}" -> "{1}" [{2}];'.format(
            edge_src, edge_dest, self.attrs_from_dict(attr),
    )

    def generate_attributes(self):
        output = []
        for section, attrs in self.graph_attributes.items():
            output.append('{0} [ {1} ];'.format(
                section, self.attrs_from_dict(attrs),
            ))
        return output
    
    # def generate_groups(self):
    #     if not self.processor.config.groups:
    #         return ''

    #     output = []
    #     for group, vertices in self.processor.groups():
    #         funcs = [vertex.name for vertex in vertices]
    #         funcs = '" "'.join(funcs)
    #         group_color = self.group_border_color.rgba_web()
    #         group_font_size = self.group_font_size
    #         output.append(
    #             'subgraph "cluster_{group}" {{ '
    #             '"{funcs}"; '
    #             'label = "{group}"; '
    #             'fontsize = "{group_font_size}"; '
    #             'fontcolor = "black"; '
    #             'style = "bold"; '
    #             'color="{group_color}"; }}'.format(**locals()))
    #     return output

    def generate_vertices(self, vertices, vertex_attrs, vertex_color_depth_attr, preserve_attrs):
        output = []
        for i in range(len(vertices)):
            vertex = vertices[i]
            if preserve_attrs != "" and vertex.attributes().__contains__(preserve_attrs) and vertex[preserve_attrs]:
                if vertex_color_depth_attr != "" and vertex.attributes().__contains__(vertex_color_depth_attr):
                    attr = {
                        'color': self.node_color_func(vertex, float(vertex[vertex_color_depth_attr])).rgba_web(),
                        'label': self.node_label_func(vertex, vertex_attrs),
                    }
                else:
                    attr = {
                        'color': self.node_color_func(vertex, 0.1).rgba_web(),
                        'label': self.node_label_func(vertex, vertex_attrs),
                    }
                self.preserve_vertices.append(vertex["id"])
                output.append(self.vertex(i, attr))

        return output

    def generate_edges(self, edges, edge_attrs, preserve_attrs):
        output = []

        for edge in edges:
            if edge.source in self.preserve_vertices and edge.target in self.preserve_vertices:
                attr = {
                    'color': self.edge_color_func(edge).rgba_web(),
                    'label': self.edge_label_func(edge, edge_attrs),
                }
                output.append(self.edge(int(edge.source), int(edge.target), attr))

        return output 


