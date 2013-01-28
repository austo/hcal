cmd_Release/hcal.node := ln -f "Release/obj.target/hcal.node" "Release/hcal.node" 2>/dev/null || (rm -rf "Release/hcal.node" && cp -af "Release/obj.target/hcal.node" "Release/hcal.node")
