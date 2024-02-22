# Setup

1. Clone the repository
2. Run: `git submodule update --init --recursive`
3. Go into `ACCL/test/refdesigns/Coyote` and checkout the `accl_integration` branch
4. Edit `finn/docker/Dockerfile.finn` and add:
```
ENV http_proxy http://proxy.ethz.ch:3128
ENV https_proxy http://proxy.ethz.ch:3128
ENV HTTPS_PROXY http://proxy.ethz.ch:3128
ENV HTTP_PROXY http://proxy.ethz.ch:3128
```
4. Edit `finn/run-docker.sh` and add:
```
if [ ! -z "$LM_LICENSE_FILE" ]; then
    DOCKER_EXEC+="-e LM_LICENSE_FILE=$LM_LICENSE_FILE "
    DOCKER_EXEC+="-e XILINXD_LICENSE_FILE=$XILINXD_LICENSE_FILE "
fi
```
5. You need to have `LM_LICENSE_FILE` and `XILINXD_LICENSE_FILE` set in your environment

# Generating the bitstreams

1. Edit the `number_of_boards` variable in `cybersecurity/build.py` to specify the number of boards you want.
2. Run: `./run-docker.sh build_custom ../cybersecurity/`


# Programming the bitstreams

1. Log into a U55C machine
2. Go into `ACCL/test/refdesigns/Coyote/driver` and run: `make` if the driver is not already compiled
3. Edit the FPGA_BIT_PATH variable in `ACCL/test/host/Coyote/run_scripts/flow_u55c.sh` to point to the appropriate bitstream
   - Standalone: `../../../../../../cybersecurity/<output_folder_name>/bitfile/cyt_top`
   - Distributed: `../../../../../../cybersecurity/<output_folder_name>/<optional_rank>/bitfile/cyt_top`
4. From ACCL/test/host/Coyote/run_scripts run: `./flow_alveo.sh 1 1`
5. Specify the id of the board that match the rank of the bitstreams you want to program

# Run the examples

## Standalone

1. Log into a U55C machine
2. Create a build folder in `ACCL/test/refdesigns/Coyote/sw`
3. From the build directory run: `/usr/bin/cmake .. -DTARGET_DIR=../../../../../software/standalone/`
4. Run make from the build directory
5. Run the executable

## Distributed

1. Log into a U55C machine
2. Make the `ACCL/test/host/Coyote/CMakeLists.txt` use `test_distributed_<number_of_boards>.cpp` as a source file instead of `test.cpp`
4. Create a build directory in `ACCL/test/host/Coyote`
5. From the build directory run: `/usr/bin/cmake .. -DMPI_C_COMPILER
=/mnt/scratch/zhe/mpich/install/bin/mpicc -DMPI_CXX_COMPILER=/mnt/scratch/zhe/mpich/install/bin/mpicxx`
6. Edit `ACCL/test/host/Coyote/run_scripts/run.sh`:
   1. Set `TEST_MODE` to 15
   2. Change `mpirun` to `/mnt/scratch/zhe/mpich/install/bin/mpirun`
   3. Change `SLEEPTIME` to 30
7. From `ACCL/test/host/Coyote/run_scripts` run: `./run.sh`
8. Specify the ids of the boards in the order of the ranks

# Address maps

The software available for the distributed case is exactly the same for the 2 and 3 boards case. The only thing that changes are the addresses. You can see the address maps for each rank in: 
- `cybersecurity/<output_folder_name>/<optional_rank>/address_map/address_map.txt`

# Troubleshooting

## Programming the boards

When reprogramming the board, the coyote_drv has to be removed/unloaded and then re-inserted. This is automatically performed by the flow script when programming. However, it might happen that the driver crashes on removal, in that case the associated board/machine cannot be used anymore. The only solution is to either hard reboot it (which only the cluster maintainers are allowed to do) or change boards. To quickly isolate the failing ones, I recommend performing the removal of the driver manually on each board before reprogramming: `sudo rmmod coyote_drv`.

## Running

Some MPI failures may happen when running `run.sh`. I have not been able to pinpoint the exact cause. I have often seen it on the 9th or 10th board but I was just able to run some examples on them, so it does not seem to be deterministic. In that case unfortunately, one has to guess which board is not working MPI and change it. Maybe a hard reboot also solves this issue, not sure.

## Testing anomalies

Just when creating this repository, I encountered some odd behaviour while testing. All data was pushed from the rank 0 board, but the rank 2 board was not able to pull anything. We only encountered two cases where the rank 2 board was blocking:

- The ACCL nodes were misconfigured and data was not sent across boards
- There was a bug in the Coyote software that prevented reading from the 3rd stream.

In both cases, since data is blocking at some point, the rank 0 board is not able to send all its data and blocks too. Which is different from what happens here as rank 0 is able to push everything. I have singled out the *board 6* as being the issue but I am really not sure about what happened there.
I have also switched from programming the boards using: ACCL/test/refdesigns/Coyote/flow_alveo.sh to using ACCL/test/host/Coyote/runs_scripts/flow_u55c.sh which may have solved the issue. The two scripts seem to perform the same steps...
