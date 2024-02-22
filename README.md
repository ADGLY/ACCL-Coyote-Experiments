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

# Generating the bitstreams

1. Edit the `number_of_boards` variable in cybersecurity/build.py to specify the number of boards you want.
2. Run: `./run-docker.sh build_custom ../cybersecurity/`


# Programming the bitstreams

1. Log into a U55C machine
2. Go into ACCL/test/refdesigns/Coyote/driver and run: `make` if the driver is not already compiled
3. Edit the FPGA_BIT_PATH variable in ACCL/test/host/Coyote/run_scripts/flow_u55c.sh to point to the appropriate bitstream
   - Standalone: ../../../../cybersecurity/<output_folder_name>/bitfile/cyt_top
   - Distributed: ../../../../cybersecurity/<output_folder_name>/<optional_rank>/bitfile/cyt_top
4. From ACCL/test/host/Coyote/run_scripts run: `./flow_alveo.sh 1 1`
5. Specify the id of the board that match the rank of the bitstreams you want to program

# Run the examples

## Standalone

1. Log into a U55C machine
2. Create a build folder in ACCL/test/refdesigns/Coyote/sw
3. From the build directory run: `/usr/bin/cmake .. -DTARGET_DIR=../../../../../software/standalone/`
4. Run make from the build directory
5. Run the executable

## Distributed

1. Log into a U55C machine
2. Make the ACCL/test/host/Coyote/CMakeLists.txt use test_distributed.cpp as a source file instead of test.cpp
4. Create a build directory in ACCL/test/host/Coyote
5. From the build directory run: `/usr/bin/cmake .. -DMPI_C_COMPILER
=/mnt/scratch/zhe/mpich/install/bin/mpicc -DMPI_CXX_COMPILER=/mnt/scratch/zhe/mpich/install/bin/mpicxx`
6. Edit ACCL/test/host/Coyote/run_scripts/run.sh:
   1. Set `TEST_MODE` to 15
   2. Change `mpirun` to `/mnt/scratch/zhe/mpich/install/bin/mpirun`
   3. Change `SLEEPTIME` to 30
7. From ACCL/test/host/Coyote/run_scripts run: `./run.sh'
8. Specify the ids of the boards in the order of the ranks

    
