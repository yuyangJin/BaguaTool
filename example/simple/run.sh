#!/bin/bash
./static_analysis ./cg.A.4
./graphperf_preprocess ./cg.A.4.pag/

dot -Tpdf -o root.pdf ./root.dot

#