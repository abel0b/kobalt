#include "kobalt/modanal.h"
#include "kobalt/modgraph.h"

static int modanal_rec(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid);

static void modanal_program(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode* node = &ast->nodes.data[nid];
    for(int i = 0; i < node->data.group.numitems; ++i) {
        modanal_rec(ast, modgraph, modid, node->data.group.items[i]);
    }
}

static void modanal_import(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) { 
    struct kbnode_import* import = (struct kbnode_import*) &ast->nodes.data[nid].data;
    struct kbstr modpath;
    kbstr_new(&modpath);
    kbstr_cat(&modpath, import->path);
    kbmodgraph_depend(modgraph, modid, &modpath);
}

static int modanal_rec(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid, int nid) {
    struct kbnode* node = &ast->nodes.data[nid];

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

void kbmodanal(struct kbast* ast, struct kbmodgraph* modgraph, struct kbstr* modid) {
    modanal_rec(ast, modgraph, modid, 0);
}
