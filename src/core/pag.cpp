#include <cstring>
#include <map>

#include "baguatool.h"
#include "igraph.h"
#include "pag.h"

namespace baguatool::core {

ProgramAbstractionGraph::ProgramAbstractionGraph() {
    ipag_ = new PAGImpl;
    // open attributes
    igraph_set_attribute_table(&igraph_cattribute_table);
}

ProgramAbstractionGraph::~ProgramAbstractionGraph() {
    igraph_destroy(&ipag_->graph);
    delete ipag_;
}

void ProgramAbstractionGraph::GraphInit(const char *graph_name) {
    // build an empty graph
    igraph_empty(&ipag_->graph, 0, IGRAPH_DIRECTED);
    // set graph name
    SETGAS(&ipag_->graph, "name", graph_name);
    // set vertex number as 0
    cur_vertex_id = 0;
}

int ProgramAbstractionGraph::AddVertex() {
    if (this->cur_vertex_id >= igraph_vcount(&ipag_->graph)) {
        igraph_add_vertices(&ipag_->graph, TRUNK_SIZE, 0);
    }
    // Add a new vertex
    igraph_integer_t new_vertex_id = this->cur_vertex_id++;

    // igraph_integer_t new_vertex_id = igraph_vcount(&ipag_.graph);
    // printf("Add a vertex: %d %s\n", new_vertex_id, vertex_name);

    // Set basic attributes

    // Return id of new vertex
    return (int)new_vertex_id;
}

int ProgramAbstractionGraph::AddEdge(const int src_vertex_id, const int dest_vertex_id) {
    // Add a new edge
    // printf("Add an edge: %d, %d\n", src_vertex_id, dest_vertex_id);
    igraph_add_edge(&ipag_->graph, (igraph_integer_t)src_vertex_id, (igraph_integer_t)dest_vertex_id);
    igraph_integer_t new_edge_id = igraph_ecount(&ipag_->graph);

    // Return id of new edge
    return (int)(new_edge_id - 1);
}

void ProgramAbstractionGraph::AddGraph(ProgramAbstractionGraph *g) {
    g->DeleteExtraTailVertices();

    std::map<int, int> old_vertex_id_2_new_vertex_id;

    // Step over all vertices
    igraph_vs_t vs;
    igraph_vit_t vit;

    igraph_vs_all(&vs);
    igraph_vit_create(&g->ipag_->graph, vs, &vit);
    while (!IGRAPH_VIT_END(vit)) {
        // Get vector id
        int vertex_id = (int)IGRAPH_VIT_GET(vit);
        printf("vertex %d", vertex_id);

        // Add new vertex (the copy of that in the input g) into this pag
        int new_vertex_id = this->AddVertex();

        old_vertex_id_2_new_vertex_id[vertex_id] = new_vertex_id;

        // copy all attributes of this vertex
        this->CopyVertex(new_vertex_id, g, vertex_id);

        IGRAPH_VIT_NEXT(vit);
    }
    printf("\n");

    igraph_vit_destroy(&vit);
    igraph_vs_destroy(&vs);

    // Step over all edges
    igraph_es_t es;
    igraph_eit_t eit;

    igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
    igraph_eit_create(&g->ipag_->graph, es, &eit);

    while (!IGRAPH_EIT_END(eit)) {
        // Get edge id
        int edge_id = (int)IGRAPH_EIT_GET(eit);
        printf("edge %d", edge_id);

        // Add new edge (the copy of that in the input g) into this pag
        int new_edge_id = this->AddEdge(old_vertex_id_2_new_vertex_id[g->GetEdgeSrc(edge_id)],
                                        old_vertex_id_2_new_vertex_id[g->GetEdgeDest(edge_id)]);

        // copy all attributes of this vertex
        // this->CopyVertex(new_vertex_id, g, vertex_id);

        IGRAPH_EIT_NEXT(eit);
    }
    printf("\n");

    igraph_eit_destroy(&eit);
    igraph_es_destroy(&es);
}

int ProgramAbstractionGraph::SetVertexBasicInfo(const int vertex_id, const int vertex_type, const char *vertex_name) {
    //
    SETVAN(&ipag_->graph, "type", vertex_id, (igraph_real_t)vertex_type);
    SETVAS(&ipag_->graph, "name", vertex_id, vertex_name);
    return 0;
}

int ProgramAbstractionGraph::SetVertexDebugInfo(const int vertex_id, const int entry_addr, const int exit_addr) {
    //
    SETVAN(&ipag_->graph, "s_addr", vertex_id, (igraph_real_t)entry_addr);
    SETVAN(&ipag_->graph, "e_addr", vertex_id, (igraph_real_t)exit_addr);
    return 0;
}

void ProgramAbstractionGraph::DeleteVertex() {}

void ProgramAbstractionGraph::DeleteEdge() {}

void ProgramAbstractionGraph::QueryVertex() {}

void ProgramAbstractionGraph::QueryEdge() {}

int ProgramAbstractionGraph::GetEdgeSrc(int edge_id) { return IGRAPH_FROM(&ipag_->graph, edge_id); }

int ProgramAbstractionGraph::GetEdgeDest(int edge_id) { return IGRAPH_TO(&ipag_->graph, edge_id); }

void ProgramAbstractionGraph::QueryEdgeOtherSide() {}

void ProgramAbstractionGraph::SetVertexAttribute() {}

void ProgramAbstractionGraph::SetEdgeAttribute() {}

void ProgramAbstractionGraph::GetVertexAttribute() {}

int ProgramAbstractionGraph::GetVertexAttributeNum(const char *attr_name, int vertex_id) {
    int ret_str = VAN(&ipag_->graph, attr_name, vertex_id);
    return ret_str;
}

const char *ProgramAbstractionGraph::GetVertexAttributeString(const char *attr_name, int vertex_id) {
    const char *ret_str = VAS(&ipag_->graph, attr_name, vertex_id);
    return ret_str;
}

void ProgramAbstractionGraph::GetEdgeAttribute() {}

const char *ProgramAbstractionGraph::GetGraphAttributeString(const char *attr_name) {
    // igraph_vector_t gtypes, vtypes, etypes;
    // igraph_strvector_t gnames, vnames, enames;
    // long int i;
    // igraph_vector_init(&gtypes, 0);
    // igraph_vector_init(&vtypes, 0);
    // igraph_vector_init(&etypes, 0);
    // igraph_strvector_init(&gnames, 0);
    // igraph_strvector_init(&vnames, 0);
    // igraph_strvector_init(&enames, 0);
    // igraph_cattribute_list(&ipag_.graph, &gnames, &gtypes, &vnames, &vtypes,
    //                          &enames, &etypes);
    // /* Graph attributes */
    // for (i = 0; i < igraph_strvector_size(&gnames); i++) {
    //     printf("%s=", STR(gnames, i));
    //     if (VECTOR(gtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
    //         igraph_real_printf(GAN(&ipag_.graph, STR(gnames, i)));
    //         putchar(' ');
    //     } else {
    //         printf("\"%s\" ", GAS(&ipag_.graph, STR(gnames, i)));
    //     }
    // }
    // printf("\n");

    const char *ret_str = GAS(&ipag_->graph, attr_name);
    return ret_str;
}

void ProgramAbstractionGraph::MergeVertices() {}

void ProgramAbstractionGraph::SplitVertex() {}

void ProgramAbstractionGraph::CopyVertex(int new_vertex_id, ProgramAbstractionGraph *g, int vertex_id) {
    igraph_vector_t gtypes, vtypes, etypes;
    igraph_strvector_t gnames, vnames, enames;
    int i;
    // igraph_vector_init(&gtypes, 0);
    igraph_vector_init(&vtypes, 0);
    // igraph_vector_init(&etypes, 0);
    // igraph_strvector_init(&gnames, 0);
    igraph_strvector_init(&vnames, 0);
    // igraph_strvector_init(&enames, 0);
    igraph_cattribute_list(&g->ipag_->graph, nullptr, nullptr, &vnames, &vtypes, nullptr, nullptr);
    /* Graph attributes */
    for (i = 0; i < igraph_strvector_size(&vnames); i++) {
        const char *vname = STR(vnames, i);
        printf("%s=", vname);
        if (VECTOR(vtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
            if (strcmp(vname, "id") == 0) {
                SETVAN(&ipag_->graph, vname, new_vertex_id, new_vertex_id);
                printf("%d", new_vertex_id);
            } else {
                SETVAN(&ipag_->graph, vname, new_vertex_id, VAN(&g->ipag_->graph, STR(vnames, i), vertex_id));
                igraph_real_printf(VAN(&g->ipag_->graph, STR(vnames, i), vertex_id));
            }

            // putchar(' ');
        } else {
            SETVAS(&ipag_->graph, vname, new_vertex_id, VAS(&g->ipag_->graph, STR(vnames, i), vertex_id));
            printf("\"%s\" ", VAS(&g->ipag_->graph, STR(vnames, i), vertex_id));
        }
    }
    printf("\n");
}

void ProgramAbstractionGraph::DeleteVertices(PAGVertex *vs) { igraph_delete_vertices(&ipag_->graph, vs->vertex); }

void ProgramAbstractionGraph::DeleteExtraTailVertices() {
    // Check the number of vertices
    PAGVertex vs;
    igraph_vs_seq(&vs.vertex, this->cur_vertex_id, igraph_vcount(&ipag_->graph) - 1);
    this->DeleteVertices(&vs);
    igraph_vs_destroy(&vs.vertex);
}

void ProgramAbstractionGraph::Dfs() {}

// void ProgramAbstractionGraph::Dfs(igraph_dfshandler_t in_callback,
// igraph_dfshandler_t out_callback) {
//   igraph_dfs(&ipag_.graph, /*root=*/0, /*mode=*/ IGRAPH_OUT,
//                /*unreachable=*/ 1, /*order=*/ 0, /*order_out=*/ 0,
//                /*father=*/ 0, /*dist=*/ 0,
//                /*in_callback=*/ in_callback, /*out_callback=*/ out_callback,
//                /*extra=*/ 0);
// }

void ProgramAbstractionGraph::ReadGraphGML(const char *file_name) {
    FILE *in_file = fopen(file_name, "r");
    igraph_read_graph_gml(&ipag_->graph, in_file);
    const char *graph_name = VAS(&ipag_->graph, "name", 0);
    SETGAS(&ipag_->graph, "name", graph_name);
    fclose(in_file);

    this->cur_vertex_id = igraph_vcount(&ipag_->graph);
}

void ProgramAbstractionGraph::DumpGraph(const char *file_name) {
    this->DeleteExtraTailVertices();

    // Real dump
    FILE *out_file = fopen(file_name, "w");
    igraph_write_graph_gml(&ipag_->graph, out_file, 0, "Bruce Jin");
    fclose(out_file);
}

void ProgramAbstractionGraph::DumpGraphDot(const char *file_name) {
    this->DeleteExtraTailVertices();

    // Real dump
    FILE *out_file = fopen(file_name, "w");
    igraph_write_graph_dot(&ipag_->graph, out_file);
    fclose(out_file);
}

void ProgramAbstractionGraph::VertexTraversal(void (*CALL_BACK_FUNC)(ProgramAbstractionGraph *, int, void *),
                                              void *extra) {
    igraph_vs_t vs;
    igraph_vit_t vit;
    printf("Function %s Start:\n", this->GetGraphAttributeString("name"));
    igraph_vs_all(&vs);
    igraph_vit_create(&ipag_->graph, vs, &vit);
    while (!IGRAPH_VIT_END(vit)) {
        // Get vector id
        int vertex_id = (int)IGRAPH_VIT_GET(vit);
        printf("Traverse %d\n", vertex_id);

        // Call user-defined function
        (*CALL_BACK_FUNC)(this, vertex_id, extra);

        IGRAPH_VIT_NEXT(vit);
    }
    printf("\n");

    igraph_vit_destroy(&vit);
    igraph_vs_destroy(&vs);
    printf("Function %s End\n", this->GetGraphAttributeString("name"));
}

int ProgramAbstractionGraph::GetCurVertexId() { return this->cur_vertex_id; }
}