options = {}
options["RNN_type"]="GRU"
options['n_hidden_output_neurons'] = 128
options['n_hidden_middle_neurons'] = 128
options['learning_rate'] = 0.00005
options['training_split'] = 0.9
options['batch_size'] = 100
options['n_batches'] = 100
options['n_layers'] = 5
options['input_size'] = 2
options['hidden_size'] = 128
options['output_size'] = 2
options['bidirectional'] = False
options['lepton_size'] = 13