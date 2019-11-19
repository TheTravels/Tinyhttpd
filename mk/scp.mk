#scp -P 22 -r ../TmpFrameServer merafour@39.108.51.99:/home/merafour/git/FrameServer                                                                                                                                                                                           

#FILE_LIST = cmake.mk FrameServer_Qt.creator FrameServer_Qt.files LICENSE   ServerConfig.cfg  start.sh  upload_tar_gz CMakeLists.txt  FrameServer_Qt.config  FrameServer_Qt.creator.user  FrameServer_Qt.includes  Makefile 
#FILE_LIST = cmake.mk FrameServer_Qt.creator FrameServer_Qt.files LICENSE  start.sh  upload_tar_gz CMakeLists.txt  FrameServer_Qt.config  FrameServer_Qt.creator.user  FrameServer_Qt.includes  Makefile 
#FILE_LIST += source
#SCP_FILE_LIST = user.txt README.md Makefile target_modules.mk
#SCP_DIR_LIST = mk modules upload

#SCP_TARGET = Tinyhttpd
#TMP_DIR = ../Tmp$(SCP_TARGET)/$(SCP_TARGET)

scp_all: scp_zdep scp_YJ                                                                                                                                                                                                                                                       
.PHONY : scp_all                                                                                                                                                                                                                                                               
#scp_zdep: clean                                                                                                                                                                                                                                                               
scp_zdep:
	@echo scp zdep
	$(call scp_func,$(SCP_HOST_ZDEP),$(SCP_PORT_ZDEP),merafour,ServerConfig_zdep3.cfg,$(SCP_TARGET),$(SCP_DIR),$(SCP_FILE_LIST),$(SCP_DIR_LIST))
.PHONY : scp_zdep
#scp_YJ: clean
scp_YJ: 
	@echo scp YJ
	$(call scp_func,yjobdc.cloudscape.net.cn,6232,obd,ServerConfig_YJ.cfg,$(SCP_TARGET),$(SCP_DIR),$(SCP_FILE_LIST),$(SCP_DIR_LIST))
.PHONY : scp_YJ

#scp_all: scp_zdep scp_YJ                                                                                                                                                                                                                                                       
#.PHONY : scp_all                                                                                                                                                                                                                                                               
#scp_zdep: clean                                                                                                                                                                                                                                                               
scp_zdep_test:
#       scp -r FrameServer merafour@39.108.51.99:/home/merafour/git                                                                                                                                                                                                            
#       scp -r . merafour@39.108.51.99:/home/merafour/git/FrameServer                                                                                                                                                                                                          
#       scp -P 22 -r ../FrameServer merafour@39.108.51.99:/home/merafour/git/FrameServer
        $(call scp_func,39.108.51.99,22,merafour,ServerConfig_zdep.cfg)
.PHONY : scp_zdep_test
#scp_YJ: clean
scp_YJ_test: 
#       scp -r -P 6232 . obd@yjobdc.cloudscape.net.cn:/home/obd/git/FrameServer
#       scp -P 6232 -r ../FrameServer obd@yjobdc.cloudscape.net.cn:/home/obd/git/FrameServer
        $(call scp_func,yjobdc.cloudscape.net.cn,6232,obd,ServerConfig_YJ.cfg)
.PHONY : scp_YJ_test


#    @echo "my name is $(0)"
#    @echo "host => $(1)"
#    @echo "port => $(2)"
#    @echo "user => $(3)"
#    @echo "file => $(4)"
define scp_func_file
    @echo "file my name is $(0)"
    @echo "file host => $(1)"
    @echo "file port => $(2)"
    @echo "file user => $(3)"
    @echo "file file => $(4)"
    scp -P $(2) ../$(SCP_TARGET)/$(4) $(3)@$(1):/home/$(3)/git/$(SCP_TARGET)/$(4)

endef
define cp_file
    @echo "file my name is $(0)"
    @echo "file  => $(1)"
    @echo "dir   => $(2)"
    cp ./$(1) $(2)/$(1)

endef
define cp_dir
    @echo "file my name is $(0)"
    @echo "copy dir  => $(1)"
    @echo "dir   => $(2)"
    cp -a ./$(1) $(2)/$(1)

endef
#    @$(foreach file,$(FILE_LIST),$(call scp_func_file,$(1),$(2),$(3),$(file)))
define scp_func
    @echo "my name is $(0)"
    @echo "host => $(1)"
    @echo "port => $(2)"
    @echo "user => $(3)"
    @echo "CFG  => $(4)"
    @echo "TARGET  => $(5)"
    @echo "SCP_DIR  => $(6)"
    @echo "SCP_FILE_LIST  => $(7)"
    @echo "SCP_DIR_LIST  => $(8)"
    rm -rf $(SCP_DIR)
    mkdir -p $(SCP_DIR)
    cp -a ./cfg/upgrade $(6)/upload
    cp ./cfg/$(4) $(6)/ServerConfig.cfg
    @$(foreach file,$(8),$(call cp_dir,$(file),$(6)))
    @$(foreach file,$(7),$(call cp_file,$(file),$(6)))
    scp -P $(2) -r $(6) $(3)@$(1):/home/$(3)/git

endef
#    cp -a ./source $(TMP_DIR)/source
#    scp -P $(2) -r ../FrameServer $(3)@$(1):/home/$(3)/git/FrameServer
#    scp -P $(2) -r ../FrameServer/source $(3)@$(1):/home/$(3)/git/FrameServer/source
#    scp -P $(2) -r $(TMP_DIR) $(3)@$(1):/home/$(3)/git/FrameServer
#    rm -rf $(TMP_DIR)

