Reference: https://gtaforums.com/topic/822314-guide-driving-styles/

The driving style is decided by a combination of the following flags.

1                              - 1         - stop before vehicles
10                             - 2         - stop before peds
100                            - 4         - avoid vehicles
1000                           ﻿- 8         - avoid empty vehicle
10000                          - 16        - avoid peds
100000                         - 32        - avoid objects
10000000                       - 128       - stop at traffic lights
100000000                      - 256       - use blinkers
1000000000                     - 512       - allow going wrong way (only does it if the correct lane is full)
10000000000                    - 1024      - go in reverse gear (backwards)
1000000000000000000            - 262144    - "Take shortest path"
10000000000000000000           - 524288    - Probably avoid offroad
10000000000000000000000        - 4194304   - Ignore roads (Uses loca﻿l pathing, only works within 200~ meters)
1000000000000000000000000      - 16777216  - "Ignore all pathing" (Goes straight to destination)﻿
100000000000000000000000000000 - 536870912 - avoid highway﻿s when possible﻿

Some examples. The script takes integer representation.

FLAGS                            - CONVERTED INTEGER - NAME/DESC OF THE DRIVING STYLE
01000000000011000000000000100101 - 1074528293        - "Rushed"
00000000000011000000000000100100 - 786468            - "Avoid Traffic"
00000000000000000000000000000110 - 6                 - "Avoid Traffic Extremely"
00000000000011000000000010101011 - 786603            - "Normal"
0000000000101100000000000010010﻿1 - 2883621           - "Ignore Lights"
00000000000000000000000000000101 - 5                 - "Sometimes Overtake Traffic"