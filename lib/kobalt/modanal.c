#include "kobalt/modanal.h"
#include "kobalt/modgraph.h"

static int modanal_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, int nid);

static void modanal_program(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, int nid) { 
    struct kl_node* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        modanal_rec(ast, modgraph, modid, node->data.group.items[i]);
    }
}

static void modanal_import(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, int nid) { 
    struct kl_node_import* import = (struct kl_node_import*) &ast->nodes.data[nid].data;
    struct abl_str modpath;
    abl_str_new(&modpath);
    abl_str_cat(&modpath, import->path);
    kl_modgraph_depend(modgraph, modid, &modpath);
}

static int modanal_rec(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid, int nid) {
    struct kl_node* node = &ast->nodes.data[nid];

    switch(node->kind) {
        case NProgram:
            modanal_program(ast, modgraph, modid, nid);
            break;
        case NImport:
			modanal_import(ast, modgraph, modid, nid);
            break;
        default:
            break;
    }
    return 1;
}

void kl_modanal(struct kl_ast* ast, struct kl_modgraph* modgraph, struct abl_str* modid) {
    modanal_rec(ast, modgraph, modid, 0);
}
