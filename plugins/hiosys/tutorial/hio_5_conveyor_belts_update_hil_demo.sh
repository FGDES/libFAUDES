# advertise
echo ========================================================
echo ===  copy tmp_hio_cb_*_controller.gen* to            ===
echo ===  data/5_conveyor_belts/hil_demo/*_controller.gen ===
echo ========================================================

DSTDIR = data/5_conveyor_belts/hil_demo

cp -v tmp_hio_cb_ABCD_controller.gen_rename $DSTDIR/ABCD_controller.gen
cp -v tmp_hio_cb_ABCDEFGH_controller.gen_rename $DSTDIR/ABCDEFGH_controller.gen
cp -v tmp_hio_cb_AB_controller.gen_rename $DSTDIR/AB_controller.gen
cp -v tmp_hio_cb_A_controller.gen $DSTDIR/A_controller.gen
cp -v tmp_hio_cb_B_controller.gen $DSTDIR/B_controller.gen
cp -v tmp_hio_cb_C_controller.gen $DSTDIR/C_controller.gen
cp -v tmp_hio_cb_CD_controller.gen_rename $DSTDIR/CD_controller.gen
cp -v tmp_hio_cb_D_controller.gen $DSTDIR/D_controller.gen
cp -v tmp_hio_cb_E_controller.gen $DSTDIR/E_controller.gen
cp -v tmp_hio_cb_EF_controller.gen_rename $DSTDIR/EF_controller.gen
cp -v tmp_hio_cb_EFGH_controller.gen_rename $DSTDIR/EFGH_controller.gen
cp -v tmp_hio_cb_F_controller.gen $DSTDIR/F_controller.gen
cp -v tmp_hio_cb_G_controller.gen $DSTDIR/G_controller.gen
cp -v tmp_hio_cb_GH_controller.gen_rename $DSTDIR/GH_controller.gen
cp -v tmp_hio_cb_H_controller.gen $DSTDIR/H_controller.gen
