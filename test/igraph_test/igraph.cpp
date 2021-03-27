#include <igraph.h>
#include <stdio.h>
#include <unistd.h>

// int main() {

//     igraph_t g1;
//     igraph_vector_t v1;
//     int ret;

//     /* Create a graph */
//     igraph_vector_init(&v1, 8);
//     VECTOR(v1)[0] = 0;
//     VECTOR(v1)[1] = 1;
//     VECTOR(v1)[2] = 1;
//     VECTOR(v1)[3] = 2;
//     VECTOR(v1)[4] = 2;
//     VECTOR(v1)[5] = 3;
//     VECTOR(v1)[6] = 2;
//     VECTOR(v1)[7] = 2;
//     igraph_create(&g1, &v1, 0, 0);
//     igraph_vector_destroy(&v1);

//     /* Add more vertices */
//     igraph_add_vertices(&g1, 10, 0);
//     if (igraph_vcount(&g1) != 14) {
//         return 1;
//     }

//     /* Add more vertices */
//     igraph_add_vertices(&g1, 0, 0);
//     if (igraph_vcount(&g1) != 14) {
//         return 2;
//     }

//     /* Error */
//     igraph_set_error_handler(igraph_error_handler_ignore);
//     ret = igraph_add_vertices(&g1, -1, 0);
//     if (ret != IGRAPH_EINVAL) {
//         return 3;
//     }

//     igraph_destroy(&g1);

//     return 0;
// }

int main() {
  igraph_t g1;
  igraph_integer_t n = 1;
  igraph_vector_t v1;
  int ret;

  igraph_set_attribute_table(&igraph_cattribute_table);

  igraph_empty(&g1, n, IGRAPH_DIRECTED);

  SETGAS(&g1, "name", "Bruce's graph");
  igraph_add_vertices(&g1, 1, 0);
  printf("%d\n", igraph_vcount(&g1));

  igraph_add_edge(&g1, 1, 0);
  SETVAS(&g1, "name", 0, "main");
  SETVAS(&g1, "name", 1, "foo");

  FILE* outfile = NULL;
  outfile = fopen("test.graphml", "w");
  // igraph_write_graph_graphml(&g1, outfile, 1);
  fclose(outfile);

  outfile = fopen("test.dat", "w");
  igraph_write_graph_gml(&g1, outfile, 0, "test suite");
  fclose(outfile);

  igraph_destroy(&g1);
  return 0;
}

// void null_warning_handler (const char *reason, const char *file,
//                            int line, int igraph_errno) {
// }

// int main() {

//     igraph_t g;
//     igraph_vector_t y;
//     igraph_warning_handler_t* oldwarnhandler;

//     /* turn on attribute handling */
//     igraph_set_attribute_table(&igraph_cattribute_table);

//     /* Create a graph, add some attributes and save it as a GraphML file */
//     igraph_famous(&g, "Petersen");
//     SETGAS(&g, "name", "Petersen's graph");
//     SETGAN(&g, "vertices", igraph_vcount(&g));
//     SETGAN(&g, "edges", igraph_ecount(&g));
//     SETGAB(&g, "famous", 1);

//     igraph_vector_init_seq(&y, 1, igraph_vcount(&g));
//     SETVANV(&g, "id", &y);
//     igraph_vector_destroy(&y);

//     SETVAS(&g, "name", 0, "foo");
//     SETVAS(&g, "name", 1, "foobar");

//     SETVAB(&g, "is_first", 0, 1);

//     igraph_vector_init_seq(&y, 1, igraph_ecount(&g));
//     SETEANV(&g, "id", &y);
//     igraph_vector_destroy(&y);

//     SETEAS(&g, "name", 0, "FOO");
//     SETEAS(&g, "name", 1, "FOOBAR");

//     SETEAB(&g, "is_first", 0, 1);

//     /* Turn off the warning handler temporarily because the GML writer will
//      * print warnings about boolean attributes being converted to numbers, and
//      * we don't care about these */
//     oldwarnhandler = igraph_set_warning_handler(null_warning_handler);
//     igraph_write_graph_gml(&g, stdout, 0, "");
//     igraph_set_warning_handler(oldwarnhandler);

//     /* Back to business */
//     FILE* file = fopen("./test.dot", "w");
//     igraph_write_graph_dot(&g, file);
//     fclose(file);

//     igraph_destroy(&g);

//     return 0;
// }