# Setup

1. Clone the repository
2. Run: `git submodule update --init --recursive`
3. Go into ACCL/test/refdesigns/Coyote and checkout the `accl_integration` branch
4. Edit finn/docker/Dockerfile.finn and add:
```
ENV http_proxy http://proxy.ethz.ch:3128
ENV https_proxy http://proxy.ethz.ch:3128
ENV HTTPS_PROXY http://proxy.ethz.ch:3128
ENV HTTP_PROXY http://proxy.ethz.ch:3128
```
4. Edit finn/run-docker.sh and add:
```
if [ ! -z "$LM_LICENSE_FILE" ]; then
    DOCKER_EXEC+="-e LM_LICENSE_FILE=$LM_LICENSE_FILE "
    DOCKER_EXEC+="-e XILINXD_LICENSE_FILE=$XILINXD_LICENSE_FILE "
fi
```
5. You need to have `LM_LICENSE_FILE` and `XILINXD_LICENSE_FILE` set in your environment
6. Edit ACCL/test/refdesigns/Coyote/flow_alveo.sh and:
   1. Change every `/opt/cli` to `/opt/sgrt/cli`.
   2. Line 100: Remove ` && ` at the end of the line
7. Edit ACCL/test/refdesigns/Coyote/sw/src/cProcess.cpp:
   1. Add `((cs_invoke.dest & CTRL_DEST_MASK) << CTRL_DEST_WR) |` after line 430 and after line 450 (or 451 if you added to the previous location). The change should look like: https://github.com/ADGLY/Coyote/commit/33fa447a2d887e5c6ede345ee60a169f602587fb

# Generating the bitstreams

1. Edit the `number_of_boards` variable in cybersecurity/build.py to specify the number of boards you want.
2. Run: `./run-docker.sh build_custom ../cybersecurity/`


# Programming the bitstreams

1. Log into a U55C machine
2. From ACCL/test/refdesigns/Coyote run: `./flow_alveo.sh <absolute_path_to_current_working_directory> ../../../../cybersecurity/<output_folder_name>/<optional_rank>/bitfile/cyt_top ./driver`
3. Specify the id of the board that match the rank of the bitstreams you want to program

# Run the examples

## Standalone

1. Log into a U55C machine
2. Create a build folder in ACCL/test/refdesigns/Coyote/sw
3. From the build directory run: `/usr/bin/cmake .. -DTARGET_DIR=../../../../../software/standalone/`
4. Run make from the build directory
5. Run the executable

## Distributed

1. Copy software/test_distributed.cpp to ACCL/test/host/Coyote
2. Make the ACCL/test/host/Coyote/CMakeLists.txt use this as a source file instead of test.cpp
3. Edit ACCL/test/host/Coyote/run_scripts/run.sh:
   1. Set `TEST_MODE` to 15
   2. Change `mpirun` to `/mnt/scratch/zhe/mpich/install/bin/mpirun`
   3. Change `SLEEPTIME` to 30
4. From ACCL/test/host/Coyote/run_scripts run: `./run.sh'
5. Specify the ids of the boards in the order of the ranks

    
