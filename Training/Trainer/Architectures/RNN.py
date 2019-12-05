# -*- coding: utf-8 -*-
"""This module uses pytorch to implement a recurrent neural network capable of
classifying prompt leptons from heavy flavor ones

Attributes:
    *

Todo:
    * test the new pack_padded_sequence implementation on gpu

"""

import torch
import torch.nn as nn
from torch.nn.utils.rnn import pack_padded_sequence, PackedSequence
import numpy as np
from torch.utils.tensorboard import SummaryWriter


def Tensor_length(track):
    """Finds the length of the non zero tensor

    Args:
        track (torch.tensor): tensor containing the events padded with zeroes at the end

    Returns:
        Length (int) of the tensor were it not zero-padded

    """
    return len(set([i[0] for i in torch.nonzero(track).numpy()]))


def hot_fixed_pack_padded_sequence(input, lengths, batch_first=False, enforce_sorted=True):
    r"""Packs a Tensor containing padded sequences of variable length.

    :attr:`input` can be of size ``T x B x *`` where `T` is the length of the
    longest sequence (equal to ``lengths[0]``), ``B`` is the batch size, and
    ``*`` is any number of dimensions (including 0). If ``batch_first`` is
    ``True``, ``B x T x *`` :attr:`input` is expected.

    For unsorted sequences, use `enforce_sorted = False`. If :attr:`enforce_sorted` is
    ``True``, the sequences should be sorted by length in a decreasing order, i.e.
    ``input[:,0]`` should be the longest sequence, and ``input[:,B-1]`` the shortest
    one. `enforce_sorted = True` is only necessary for ONNX export.

    Note:
        This function accepts any input that has at least two dimensions. You
        can apply it to pack the labels, and use the output of the RNN with
        them to compute the loss directly. A Tensor can be retrieved from
        a :class:`PackedSequence` object by accessing its ``.data`` attribute.

    Arguments:
        input (Tensor): padded batch of variable length sequences.
        lengths (Tensor): list of sequences lengths of each batch element.
        batch_first (bool, optional): if ``True``, the input is expected in ``B x T x *``
            format.
        enforce_sorted (bool, optional): if ``True``, the input is expected to
            contain sequences sorted by length in a decreasing order. If
            ``False``, this condition is not checked. Default: ``True``.

    Returns:
        a :class:`PackedSequence` object
    """
    if torch._C._get_tracing_state() and not isinstance(lengths, torch.Tensor):
        warnings.warn('pack_padded_sequence has been called with a Python list of '
                      'sequence lengths. The tracer cannot track the data flow of Python '
                      'values, and it will treat them as constants, likely rendering '
                      'the trace incorrect for any other combination of lengths.',
                      category=torch.jit.TracerWarning, stacklevel=2)
    lengths = torch.as_tensor(lengths, dtype=torch.int64, device = "cpu")
    #lengths = lengths.cpu()
    if enforce_sorted:
        sorted_indices = None
    else:
        lengths, sorted_indices = torch.sort(lengths, descending=True)
        sorted_indices = sorted_indices.to(input.device)
        batch_dim = 0 if batch_first else 1
        input = input.index_select(batch_dim, sorted_indices)

    data, batch_sizes = torch._C._VariableFunctions._pack_padded_sequence(input, lengths, batch_first)
    return PackedSequence(data, batch_sizes, sorted_indices)


class Model(nn.Module):
    """Model class implementing rnn inheriting structure from pytorch nn module

    Attributes:
        options (dict) : configuration for the nn

    Methods:
        forward: steps through the neural net once
        accuracy: compares predicted values to true values
        do_train: takes in data and passes the batches to forward to train
        do_eval: runs the neural net on the data after setting it up for evaluation
        get_model: returns the model and its optimizer

    """

    def __init__(self, options):
        super().__init__()
        self.n_directions = int(options["bidirectional"]) + 1
        self.n_layers = options["n_layers"]
        self.input_size = options["track_size"]
        self.hidden_size = options["hidden_neurons"]
        self.lepton_size = options["lepton_size"]
        self.output_size = options["output_neurons"]
        self.learning_rate = options["learning_rate"]
        self.batch_size = options["batch_size"]
        self.history_logger = SummaryWriter(options["output_folder"])
        self.device = options["device"]
        self.h_0 = nn.Parameter(
            torch.zeros(
                self.n_layers * self.n_directions, self.batch_size, self.hidden_size
            ).to(self.device)
        )
        self.cellstate = False  # set to true only if lstm

        if options["RNN_type"] == "RNN":
            self.rnn = nn.RNN(
                input_size=self.input_size,
                hidden_size=self.hidden_size,
                batch_first=True,
                num_layers=self.n_layers,
                bidirectional=options["bidirectional"],
            ).to(self.device)
        elif options["RNN_type"] == "LSTM":
            self.cellstate = True
            self.rnn = nn.LSTM(
                input_size=self.input_size,
                hidden_size=self.hidden_size,
                batch_first=True,
                num_layers=self.n_layers,
                bidirectional=options["bidirectional"],
            ).to(self.device)
        else:
            self.rnn = nn.GRU(
                input_size=self.input_size,
                hidden_size=self.hidden_size,
                batch_first=True,
                num_layers=self.n_layers,
                bidirectional=options["bidirectional"],
            ).to(self.device)

        self.fc = nn.Linear(self.hidden_size + self.lepton_size , self.output_size).to(self.device)
        self.softmax = nn.Softmax(dim=1).to(self.device)
        self.loss_function = nn.BCEWithLogitsLoss()
        self.optimizer = torch.optim.Adam(
            self.parameters(), lr=self.learning_rate)

    def forward(self, padded_seq, sorted_leptons):
        """Takes a padded sequence and passes it through:
            * the rnn cell
            * a fully connected layer to get it to the right output size
            * a softmax to get a probability

        Args:
            padded_seq (paddedSequence): a collection for lepton track information
            sorted_leptons : lepton features to add after rnn

        Returns:
           the probability of particle beng prompt or heavy flavor

        """
        self.rnn.flatten_parameters()
        if self.cellstate:
            output, hidden, cellstate = self.rnn(padded_seq, self.h_0)
        else:
            output, hidden = self.rnn(padded_seq, self.h_0)

        combined_out = torch.cat((sorted_leptons, hidden[-1]), dim = 1).to(self.device)
        out = self.fc(combined_out).to(self.device)
        out = self.softmax(out).to(self.device)
        return out

    def accuracy(self, predicted, truth):
        """Compares the predicted values to the true values

        Args:
            predicted (torch.tensor): predictions from the neural net

        Returns:
            normalized number of accurate predictions

        """
        return torch.from_numpy(
            np.array((predicted == truth.float()).sum().float() / len(truth))
        )

    def do_train(self, batches, do_training=True):
        """runs the neural net on batches of data passed into it

        Args:
            batches (torch.dataset object): Shuffled samples of data for evaluation by the model
                                            contains:
                                                * track_info
                                                * lepton_info
                                                * truth
            do_training (bool, True by default): flags whether the model is to be run in
                                                training or evaluation mode

        Returns: total loss, total accuracy, raw results, and all truths

        Notes:
            indices have been removed
            I don't know how the new pack-pad-sequeces works yet

        """
        if do_training:
            self.rnn.train()
        else:
            self.rnn.eval()
        total_loss = 0
        total_acc = 0
        raw_results = []
        all_truth = []

        for i, batch in enumerate(batches, 1):
            self.optimizer.zero_grad()
            track_info, lepton_info, truth = batch

            # moving tensors to adequate device
            track_info = track_info.to(self.device)
            lepton_info = lepton_info.to(self.device)
            truth = truth[:, 0].to(self.device)

            # setting up for packing padded sequence
            n_tracks = torch.tensor(
                [Tensor_length(track_info[i]) for i in range(len(track_info))]
            ).cpu()


            sorted_n, indices = torch.sort(n_tracks, descending=True)
            # reodering information according to sorted indices
            # if padding is required, this should be changed to make sorting more efficient
            sorted_tracks = track_info[indices].to(self.device)
            sorted_leptons = lepton_info[indices].to(self.device)
            ## padding sequences - turned off for now, due to 0-track leptons
            # padded_seq = hot_fixed_pack_padded_sequence(
                # track_info, n_tracks.cpu(), batch_first=True, enforce_sorted=False)
            padded_seq = sorted_tracks

            output = self.forward(padded_seq, sorted_leptons).to(self.device)
            loss = self.loss_function(output[:, 0], truth.float())

            if do_training is True:
                loss.backward()
                self.optimizer.step()
            total_loss += float(loss)
            predicted = torch.round(output)[:, 0]
            accuracy = float(
                self.accuracy(
                    predicted.data.cpu().detach(), truth.data.cpu().detach()
                )
            )
            total_acc += accuracy
            raw_results += output[:, 0].cpu().detach().tolist()
            all_truth += truth.cpu().detach().tolist()
            if do_training is True:
                self.history_logger.add_scalar(
                    "Accuracy/Train Accuracy", accuracy, i)
                self.history_logger.add_scalar(
                    "Loss/Train Loss", float(loss), i)
            else:
                self.history_logger.add_scalar(
                    "Accuracy/Test Accuracy", accuracy, i)
                self.history_logger.add_scalar(
                    "Loss/Test Loss", float(loss), i)
            for name, param in self.named_parameters():
                self.history_logger.add_histogram(
                    name, param.clone().cpu().data.numpy(), i
                )

        total_loss = total_loss / len(batches.dataset) * self.batch_size
        total_acc = total_acc / len(batches.dataset) * self.batch_size
        total_loss = torch.tensor(total_loss)
        return total_loss, total_acc, raw_results, all_truth

    def do_eval(self, batches, do_training=False):
        """Convienience function for running do_train in evaluation mode

        Args:
            batches (torch.dataset object): Shuffled samples of data for evaluation by the model
                                            contains:
                                                * track_info
                                                * lepton_info
                                                * truth
            do_training (bool, False by default): flags whether the model is to be run in
                                                training or evaluation mode

        Returns: total loss, total accuracy, raw results, and all truths

        """
        return self.do_train(batches, do_training=False)

    def get_model(self):
        """ getter function to help easy storage of the model

        Args:
            None

        Returns: the model and its optimizer

        """
        return self, self.optimizer
