# #!/bin/bash

# PROGRAM="./lb3/2"

# run_test() {
#     local FLAG=$1
#     local SIGNAL=$2
#     local COUNT=$3
#     "$PROGRAM" "$FLAG" &
#     PID=$!
#     sleep 1
    
#     for ((i=0; i<COUNT; i++)); do
#         kill -"$SIGNAL" "$PID"
#         # sleep 0.1
#     done
    
#     # sleep 1
#     # kill "$PID" 2>/dev/null
#     # wait "$PID" 2>/dev/null
# }

# run_test "--process_single" SIGINT 2
# run_test "--process_multi" SIGINT 4
# run_test "--thread_single" SIGINT 2
# run_test "--thread_multi" SIGINT 4
# run_test "--custom_signal" SIGTSTP 1