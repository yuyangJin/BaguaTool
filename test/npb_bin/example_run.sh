#!/bin/bash

$(BAGUA_DIR)/bin/static_analysis ./cg.A.4
$(BAGUA_DIR)/graphperf_preprocess ./cg.A.4.pag/

dot -Tpdf -o root.pdf ./root.dot

#