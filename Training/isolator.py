# -*- coding: utf-8 -*-
"""Driver function for running the neural network

Attributes:
    *--disable-cuda : runs the code only on cpu even if gpu is available
    *--continue-training : loads in a previous model to continue training
"""
from Trainer import trainer as trainer
from os import path
import argparse
import torch
import time


torch.backends.cudnn.benchmark = True
torch.backends.cudnn.enabled = True

parser = argparse.ArgumentParser(description="Trainer")
parser.add_argument("--disable-cuda", action="store_true", help="Disable CUDA")
parser.add_argument(
    "--continue-training",
    action="store_true",
    help="Loads in previous model and continues training",
)
args = parser.parse_args()
args.device = None
if not args.disable_cuda and torch.cuda.is_available():
    args.device = torch.device("cuda")
    torch.set_default_tensor_type(torch.cuda.FloatTensor)
else:
    args.device = torch.device("cpu")


if __name__ == "__main__":

    options = {}

    options["input_data"] = "/public/data/RNN/Samples/InclusivePt/large_data.root"
    #options["input_data"] = "/home/cunweifan/data/slice_small_data.root"  #"/public/data/RNN/Samples/InclusivePt/large_data.root"
    assert path.exists(options["input_data"]) is True, "invalid input_data path"
    options["run_location"] = "/home/cunweifan/runs"
    options["run_label"] = "SetTransformer_large_data"
    options["tree_name"] = "NormalizedTree"
    options["output_folder"] = "./Outputs/"
    options["model_path"] = options["output_folder"] + "saved_model.pt"
    options["continue_training"] = args.continue_training
    options["architecture_type"] = "SetTransformer"  # RNN, LSTM, GRU, DeepSets, SetTransformer
    options["dropout"] = 0.3
    options[
        "track_ordering"
    ] = "low-to-high-pt"  # None, "high-to-low-pt", "low-to-high-pt", "near-to-far", "far-to-near"
    # options["additional_appended_features"] = ["baseline_topoetcone20", "baseline_topoetcone30", "baseline_topoetcone40", "baseline_eflowcone20", "baseline_ptcone20", "baseline_ptcone30", "baseline_ptcone40", "baseline_ptvarcone20", "baseline_ptvarcone30", "baseline_ptvarcone40"]
    options["additional_appended_features"] = []
    options["lr"] = 0.001   #0.001
    options["decay_period"] = 30
    options["decay_rate"] = 0.1
    options["ignore_features"] = [
        "baseline_topoetcone20",
        "baseline_topoetcone30",
        "baseline_topoetcone40",
        "baseline_eflowcone20",
        "baseline_ptcone20",
        "baseline_ptcone30",
        "baseline_ptcone40",
        "baseline_ptvarcone20",
        "baseline_ptvarcone30",
        "baseline_ptvarcone40",
        "baseline_eflowcone20_over_pt",
        "trk_vtx_type",
    ]
    options["training_split"] = 0.7
    options["batch_size"] = 32
    options["n_epochs"] = 100
    options["n_layers"] = 3
    options["hidden_neurons"] = 256
    options["intrinsic_dimensions"] = 1024  # only matters for deep sets
    options["output_neurons"] = 2
    options["device"] = args.device
    options["save_model"] = True
    options["model_save_path"] = options["output_folder"] + "test_gru_model.pth"
    options["train_BDT"] = True
    t0 = time.time()
    print("number of epochs planned:", options["n_epochs"])
    print("input data:", options["input_data"].split("/")[-1])
    print("batch_size:", options["batch_size"])
    print("device:", args.device)
    print("architecture:", options["architecture_type"])
    trainer.train(options)
    print("total runtime :", time.time() - t0)
    torch.cuda.empty_cache()
