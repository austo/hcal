cmd_Release/hcal.node := ./gyp-mac-tool flock ./Release/linker.lock c++ -shared -Wl,-search_paths_first -mmacosx-version-min=10.5 -arch x86_64 -L./Release -install_name @rpath/hcal.node  -o Release/hcal.node Release/obj.target/hcal/hcal.o Release/obj.target/hcal/eventWrapper.o Release/obj.target/hcal/event.o Release/obj.target/hcal/eventWriter.o -undefined dynamic_lookup -lhpdf -lboost_date_time
