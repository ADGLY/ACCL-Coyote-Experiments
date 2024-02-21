from qonnx.custom_op.registry import getCustomOp
import finn.builder.build_dataflow as build
import finn.builder.build_dataflow_config as build_cfg
import time


number_of_boards: int = 1

# Simple function to partition the example model from cybersecurity. For a
# larger model finn-experimental.utils.partitioning could be used in place of
# this.
def step_partition_model(model, cfg):
    for i, node in enumerate(model.graph.node):
        node_inst = getCustomOp(node)
        # This distributes the model to two boards
        if number_of_boards == 2:
            node_inst.set_nodeattr("device_id", 0 if i < 2 else 1)
        else:
            # This would distribute it across three boards
            # NOTE: The i < 2 and i < 6 conditions are subject to change depending on the version of FINN
            # NOTE: For instance, if the newest version insert nodes for whatever reason in the design the conditions
            # will change
            node_inst.set_nodeattr("device_id", 0 if i < 2 else 1 if i < 6 else 2)
    return model

# Define custom build flow
steps = None
if number_of_boards > 1:
    steps = [
        "step_qonnx_to_finn",
        "step_tidy_up",
        "step_streamline",
        "step_convert_to_hls",
        "step_create_dataflow_partition",
        "step_target_fps_parallelization",
        "step_apply_folding_config",
        "step_minimize_bit_width",
        "step_generate_estimate_reports",
        "step_hls_codegen",
        "step_hls_ipgen",
        "step_set_fifo_depths",
        step_partition_model,
        "step_insert_accl",
        "step_split_dataflow",
        "step_create_stitched_ip",
        "step_measure_rtlsim_performance",
        "step_out_of_context_synthesis",
        "step_synthesize_bitfile",
        "step_make_pynq_driver",
        "step_deployment_package",
    ]

output_dir_name = ""
if number_of_boards == 1:
    output_dir_name = "output_standalone"
else
    output_dir_name = f"output_{number_of_boards}_boards"

model_file = "./cybsec-mlp-ready.onnx"
output_dir = f"./{output_dir_name}"

cfg_build = build.DataflowBuildConfig(
    output_dir=output_dir,
    mvau_wwidth_max=80,
    target_fps=1000000,
    synth_clk_period_ns=6.66,
    board="U55C",
    shell_flow_type=build_cfg.ShellFlowType.COYOTE_ALVEO,
    steps=steps,
    start_step=None,
    save_intermediate_models=True,
    generate_outputs=[
        build_cfg.DataflowOutputType.BITFILE],
    verbose=False,
    folding_config_file="./custom_config.json",
    stitched_ip_gen_dcp=True
)

step_start = time.time()
if number_of_boards > 1:
    build.build_distributed_dataflow_cfg(model_file, cfg_build)
else:
    build.build_dataflow_cfg(model_file, cfg_build)
step_end = time.time()
print(f"Build took {step_end - step_start}")
