/*
   Fast Artificial Neural Network Library (fann)
   Copyright (C) 2003-2016 Steffen Nissen (steffen.fann@gmail.com)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "config.h"
#include "fann.h"

//
// Reads training data from a file.
//
FANN_EXTERNAL FannTrainData * FANN_API fann_read_train_from_file(const char * configuration_file)
{
	FannTrainData * data = 0;
	FILE * file = fopen(configuration_file, "r");
	if(!file) {
		fann_error(NULL, FANN_E_CANT_OPEN_CONFIG_R, configuration_file);
	}
	else {
		data = fann_read_train_from_fd(file, configuration_file);
		fclose(file);
	}
	return data;
}
/*
 * Save training data to a file
 */
FANN_EXTERNAL int FANN_API fann_save_train(FannTrainData * data, const char * filename)
{
	return fann_save_train_internal(data, filename, 0, 0);
}

/*
 * Save training data to a file in fixed point algebra. (Good for testing
 * a network in fixed point)
 */
FANN_EXTERNAL int FANN_API fann_save_train_to_fixed(FannTrainData * data, const char * filename, uint decimal_point)
{
	return fann_save_train_internal(data, filename, 1, decimal_point);
}

/*
 * deallocate the train data structure.
 */
FANN_EXTERNAL void FANN_API fann_destroy_train(FannTrainData * data)
{
	if(data == NULL)
		return;
	if(data->input != NULL)
		ZFREE(data->input[0]);
	if(data->output != NULL)
		ZFREE(data->output[0]);
	ZFREE(data->input);
	ZFREE(data->output);
	ZFREE(data);
}

/*
 * Test a set of training data and calculate the MSE
 */
FANN_EXTERNAL float FANN_API fann_test_data(Fann * ann, FannTrainData * data)
{
	uint i;
	if(fann_check_input_output_sizes(ann, data) == -1)
		return 0;
	fann_reset_MSE(ann);
	for(i = 0; i != data->num_data; i++) {
		fann_test(ann, data->input[i], data->output[i]);
	}
	return fann_get_MSE(ann);
}

#ifndef FIXEDFANN

/*
 * Internal train function
 */
float fann_train_epoch_quickprop(Fann * ann, FannTrainData * data)
{
	uint i;
	if(ann->prev_train_slopes == NULL) {
		fann_clear_train_arrays(ann);
	}
	fann_reset_MSE(ann);
	for(i = 0; i < data->num_data; i++) {
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->P_FirstLayer + 1, ann->P_LastLayer-1);
	}
	fann_update_weights_quickprop(ann, data->num_data, 0, ann->total_connections);
	return fann_get_MSE(ann);
}

/*
 * Internal train function
 */
float fann_train_epoch_irpropm(Fann * ann, FannTrainData * data)
{
	uint i;
	if(ann->prev_train_slopes == NULL) {
		fann_clear_train_arrays(ann);
	}
	fann_reset_MSE(ann);
	for(i = 0; i < data->num_data; i++) {
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->P_FirstLayer + 1, ann->P_LastLayer-1);
	}
	fann_update_weights_irpropm(ann, 0, ann->total_connections);
	return fann_get_MSE(ann);
}
/*
 * Internal train function
 */
float fann_train_epoch_sarprop(Fann * ann, FannTrainData * data)
{
	uint i;
	if(ann->prev_train_slopes == NULL) {
		fann_clear_train_arrays(ann);
	}
	fann_reset_MSE(ann);
	for(i = 0; i < data->num_data; i++) {
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->P_FirstLayer + 1, ann->P_LastLayer-1);
	}
	fann_update_weights_sarprop(ann, ann->sarprop_epoch, 0, ann->total_connections);
	++(ann->sarprop_epoch);
	return fann_get_MSE(ann);
}

/*
 * Internal train function
 */
float fann_train_epoch_batch(Fann * ann, FannTrainData * data)
{
	uint i;
	fann_reset_MSE(ann);
	for(i = 0; i < data->num_data; i++) {
		fann_run(ann, data->input[i]);
		fann_compute_MSE(ann, data->output[i]);
		fann_backpropagate_MSE(ann);
		fann_update_slopes_batch(ann, ann->P_FirstLayer + 1, ann->P_LastLayer-1);
	}
	fann_update_weights_batch(ann, data->num_data, 0, ann->total_connections);
	return fann_get_MSE(ann);
}
/*
 * Internal train function
 */
float fann_train_epoch_incremental(Fann * ann, FannTrainData * data)
{
	fann_reset_MSE(ann);
	for(uint i = 0; i != data->num_data; i++) {
		fann_train(ann, data->input[i], data->output[i]);
	}
	return fann_get_MSE(ann);
}

/*
 * Train for one epoch with the selected training algorithm
 */
FANN_EXTERNAL float FANN_API fann_train_epoch(Fann * ann, FannTrainData * data)
{
	if(fann_check_input_output_sizes(ann, data) == -1)
		return 0;
	switch(ann->training_algorithm) {
		case FANN_TRAIN_QUICKPROP:   return fann_train_epoch_quickprop(ann, data);
		case FANN_TRAIN_RPROP:       return fann_train_epoch_irpropm(ann, data);
		case FANN_TRAIN_SARPROP:     return fann_train_epoch_sarprop(ann, data);
		case FANN_TRAIN_BATCH:       return fann_train_epoch_batch(ann, data);
		case FANN_TRAIN_INCREMENTAL: return fann_train_epoch_incremental(ann, data);
	}
	return 0;
}

FANN_EXTERNAL void FANN_API fann_train_on_data(Fann * ann, FannTrainData * data,
    uint max_epochs, uint epochs_between_reports, float desired_error)
{
	float error;
	uint i;
	int desired_error_reached;
#ifdef DEBUG
	printf("Training with %s\n", FANN_TRAIN_NAMES[ann->training_algorithm]);
#endif
	if(epochs_between_reports && ann->callback == NULL) {
		printf("Max epochs %8d. Desired error: %.10f.\n", max_epochs, desired_error);
	}
	for(i = 1; i <= max_epochs; i++) {
		/*
		 * train
		 */
		error = fann_train_epoch(ann, data);
		desired_error_reached = fann_desired_error_reached(ann, desired_error);
		/*
		 * print current output
		 */
		if(epochs_between_reports && (i % epochs_between_reports == 0 || i == max_epochs || i == 1 || desired_error_reached == 0)) {
			if(ann->callback == NULL) {
				printf("Epochs     %8d. Current error: %.10f. Bit fail %d.\n", i, error, ann->num_bit_fail);
			}
			else if(((*ann->callback)(ann, data, max_epochs, epochs_between_reports, desired_error, i)) == -1) {
				/*
				 * you can break the training by returning -1
				 */
				break;
			}
		}
		if(desired_error_reached == 0)
			break;
	}
}

FANN_EXTERNAL void FANN_API fann_train_on_file(Fann * ann, const char * filename,
    uint max_epochs, uint epochs_between_reports, float desired_error)
{
	FannTrainData * data = fann_read_train_from_file(filename);
	if(data) {
		fann_train_on_data(ann, data, max_epochs, epochs_between_reports, desired_error);
		fann_destroy_train(data);
	}
}

#endif

/*
 * shuffles training data, randomizing the order
 */
FANN_EXTERNAL void FANN_API fann_shuffle_train_data(FannTrainData * train_data)
{
	for(uint dat = 0; dat < train_data->num_data; dat++) {
		uint   swap = (uint)(rand() % train_data->num_data);
		if(swap != dat) {
			uint   elem;
			for(elem = 0; elem < train_data->num_input; elem++) {
				const fann_type temp = train_data->input[dat][elem];
				train_data->input[dat][elem] = train_data->input[swap][elem];
				train_data->input[swap][elem] = temp;
			}
			for(elem = 0; elem < train_data->num_output; elem++) {
				const fann_type temp = train_data->output[dat][elem];
				train_data->output[dat][elem] = train_data->output[swap][elem];
				train_data->output[swap][elem] = temp;
			}
		}
	}
}
/*
 * INTERNAL FUNCTION calculates min and max of train data
 */
void fann_get_min_max_data(fann_type ** data, uint num_data, uint num_elem, fann_type * min, fann_type * max)
{
	fann_type temp;
	uint dat, elem;
	*min = *max = data[0][0];
	for(dat = 0; dat < num_data; dat++) {
		for(elem = 0; elem < num_elem; elem++) {
			temp = data[dat][elem];
			if(temp < *min)
				*min = temp;
			else if(temp > *max)
				*max = temp;
		}
	}
}

FANN_EXTERNAL fann_type FANN_API fann_get_min_train_input(FannTrainData * train_data)
{
	fann_type min, max;
	fann_get_min_max_data(train_data->input, train_data->num_data, train_data->num_input, &min, &max);
	return min;
}

FANN_EXTERNAL fann_type FANN_API fann_get_max_train_input(FannTrainData * train_data)
{
	fann_type min, max;
	fann_get_min_max_data(train_data->input, train_data->num_data, train_data->num_input, &min, &max);
	return max;
}

FANN_EXTERNAL fann_type FANN_API fann_get_min_train_output(FannTrainData * train_data)
{
	fann_type min, max;
	fann_get_min_max_data(train_data->output, train_data->num_data, train_data->num_output, &min, &max);
	return min;
}

FANN_EXTERNAL fann_type FANN_API fann_get_max_train_output(FannTrainData * train_data)
{
	fann_type min, max;
	fann_get_min_max_data(train_data->output, train_data->num_data, train_data->num_output, &min, &max);
	return max;
}

/*
 * INTERNAL FUNCTION Scales data to a specific range
 */
void fann_scale_data(fann_type ** data, uint num_data, uint num_elem, fann_type new_min, fann_type new_max)
{
	fann_type old_min, old_max;
	fann_get_min_max_data(data, num_data, num_elem, &old_min, &old_max);
	fann_scale_data_to_range(data, num_data, num_elem, old_min, old_max, new_min, new_max);
}

/*
 * INTERNAL FUNCTION Scales data to a specific range
 */
FANN_EXTERNAL void FANN_API fann_scale_data_to_range(fann_type ** data, uint num_data, uint num_elem,
    fann_type old_min, fann_type old_max, fann_type new_min, fann_type new_max)
{
	const fann_type old_span = old_max - old_min;
	const fann_type new_span = new_max - new_min;
	const fann_type factor = new_span / old_span;
	// printf("max %f, min %f, factor %f\n", old_max, old_min, factor);
	for(uint dat = 0; dat < num_data; dat++) {
		for(uint elem = 0; elem < num_elem; elem++) {
			const fann_type temp = (data[dat][elem] - old_min) * factor + new_min;
			if(temp < new_min) {
				data[dat][elem] = new_min;
				// printf("error %f < %f\n", temp, new_min);
			}
			else if(temp > new_max) {
				data[dat][elem] = new_max;
				// printf("error %f > %f\n", temp, new_max);
			}
			else {
				data[dat][elem] = temp;
			}
		}
	}
}

/*
 * Scales the inputs in the training data to the specified range
 */
FANN_EXTERNAL void FANN_API fann_scale_input_train_data(FannTrainData * train_data, fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->input, train_data->num_data, train_data->num_input, new_min, new_max);
}
/*
 * Scales the inputs in the training data to the specified range
 */
FANN_EXTERNAL void FANN_API fann_scale_output_train_data(FannTrainData * train_data, fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->output, train_data->num_data, train_data->num_output, new_min, new_max);
}
/*
 * Scales the inputs in the training data to the specified range
 */
FANN_EXTERNAL void FANN_API fann_scale_train_data(FannTrainData * train_data, fann_type new_min, fann_type new_max)
{
	fann_scale_data(train_data->input, train_data->num_data, train_data->num_input, new_min, new_max);
	fann_scale_data(train_data->output, train_data->num_data, train_data->num_output, new_min, new_max);
}

/*
 * merges training data into a single struct.
 */
FANN_EXTERNAL FannTrainData * FANN_API fann_merge_train_data(FannTrainData * data1, FannTrainData * data2)
{
	uint i;
	fann_type * data_input, * data_output;
	FannTrainData * dest = (FannTrainData*)malloc(sizeof(FannTrainData));
	if(dest == NULL) {
		fann_error((FannError*)data1, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}
	if((data1->num_input != data2->num_input) || (data1->num_output != data2->num_output)) {
		fann_error((FannError*)data1, FANN_E_TRAIN_DATA_MISMATCH);
		return NULL;
	}
	fann_init_error_data((FannError*)dest);
	dest->error_log = data1->error_log;
	dest->num_data = data1->num_data+data2->num_data;
	dest->num_input = data1->num_input;
	dest->num_output = data1->num_output;
	dest->input = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL) {
		fann_error((FannError*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	dest->output = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL) {
		fann_error((FannError*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	data_input = (fann_type*)calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL) {
		fann_error((FannError*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data1->input[0], dest->num_input * data1->num_data * sizeof(fann_type));
	memcpy(data_input + (dest->num_input*data1->num_data),
	    data2->input[0], dest->num_input * data2->num_data * sizeof(fann_type));
	data_output = (fann_type*)calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL) {
		fann_error((FannError*)data1, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data1->output[0], dest->num_output * data1->num_data * sizeof(fann_type));
	memcpy(data_output + (dest->num_output*data1->num_data),
	    data2->output[0], dest->num_output * data2->num_data * sizeof(fann_type));
	for(i = 0; i != dest->num_data; i++) {
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

/*
 * return a copy of a fann_train_data struct
 */
FANN_EXTERNAL FannTrainData * FANN_API fann_duplicate_train_data(FannTrainData * data)
{
	uint i;
	fann_type * data_input, * data_output;
	FannTrainData * dest = (FannTrainData*)malloc(sizeof(FannTrainData));
	if(dest == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}
	fann_init_error_data((FannError*)dest);
	dest->error_log = data->error_log;
	dest->num_data = data->num_data;
	dest->num_input = data->num_input;
	dest->num_output = data->num_output;
	dest->input = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	dest->output = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	data_input = (fann_type*)calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data->input[0], dest->num_input * dest->num_data * sizeof(fann_type));
	data_output = (fann_type*)calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data->output[0], dest->num_output * dest->num_data * sizeof(fann_type));
	for(i = 0; i != dest->num_data; i++) {
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

FANN_EXTERNAL FannTrainData * FANN_API fann_subset_train_data(FannTrainData * data, uint pos, uint length)
{
	uint i;
	fann_type * data_input, * data_output;
	FannTrainData * dest = (FannTrainData*)malloc(sizeof(FannTrainData));
	if(dest == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}
	if(pos > data->num_data || pos+length > data->num_data) {
		fann_error((FannError*)data, FANN_E_TRAIN_DATA_SUBSET, pos, length, data->num_data);
		return NULL;
	}
	fann_init_error_data((FannError*)dest);
	dest->error_log = data->error_log;
	dest->num_data = length;
	dest->num_input = data->num_input;
	dest->num_output = data->num_output;
	dest->input = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->input == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	dest->output = (fann_type**)calloc(dest->num_data, sizeof(fann_type *));
	if(dest->output == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	data_input = (fann_type*)calloc(dest->num_input * dest->num_data, sizeof(fann_type));
	if(data_input == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_input, data->input[pos], dest->num_input * dest->num_data * sizeof(fann_type));
	data_output = (fann_type*)calloc(dest->num_output * dest->num_data, sizeof(fann_type));
	if(data_output == NULL) {
		fann_error((FannError*)data, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(dest);
		return NULL;
	}
	memcpy(data_output, data->output[pos], dest->num_output * dest->num_data * sizeof(fann_type));
	for(i = 0; i != dest->num_data; i++) {
		dest->input[i] = data_input;
		data_input += dest->num_input;
		dest->output[i] = data_output;
		data_output += dest->num_output;
	}
	return dest;
}

FANN_EXTERNAL uint FANN_API fann_length_train_data(FannTrainData * data)
{
	return data->num_data;
}

FANN_EXTERNAL uint FANN_API fann_num_input_train_data(FannTrainData * data)
{
	return data->num_input;
}

FANN_EXTERNAL uint FANN_API fann_num_output_train_data(FannTrainData * data)
{
	return data->num_output;
}

/* INTERNAL FUNCTION
   Save the train data structure.
 */
int fann_save_train_internal(FannTrainData * data, const char * filename,
    uint save_as_fixed, uint decimal_point)
{
	int retval = 0;
	FILE * file = fopen(filename, "w");
	if(!file) {
		fann_error((FannError*)data, FANN_E_CANT_OPEN_TD_W, filename);
		return -1;
	}
	retval = fann_save_train_internal_fd(data, file, filename, save_as_fixed, decimal_point);
	fclose(file);
	return retval;
}

/* INTERNAL FUNCTION
   Save the train data structure.
 */
int fann_save_train_internal_fd(FannTrainData * data, FILE * file, const char * filename,
    uint save_as_fixed, uint decimal_point)
{
	uint num_data = data->num_data;
	uint num_input = data->num_input;
	uint num_output = data->num_output;
	uint i, j;
	int retval = 0;
#ifndef FIXEDFANN
	uint multiplier = 1 << decimal_point;
#endif
	fprintf(file, "%u %u %u\n", data->num_data, data->num_input, data->num_output);
	for(i = 0; i < num_data; i++) {
		for(j = 0; j < num_input; j++) {
#ifndef FIXEDFANN
			if(save_as_fixed) {
				fprintf(file, "%d ", (int)(data->input[i][j] * multiplier));
			}
			else{
				if(((int)floor(data->input[i][j] + 0.5) * 1000000) == ((int)floor(data->input[i][j] * 1000000.0 + 0.5))) {
					fprintf(file, "%d ", (int)data->input[i][j]);
				}
				else {
					fprintf(file, "%f ", data->input[i][j]);
				}
			}
#else
			fprintf(file, FANNPRINTF " ", data->input[i][j]);
#endif
		}
		fprintf(file, "\n");

		for(j = 0; j < num_output; j++) {
#ifndef FIXEDFANN
			if(save_as_fixed) {
				fprintf(file, "%d ", (int)(data->output[i][j] * multiplier));
			}
			else{
				if(((int)floor(data->output[i][j] + 0.5) * 1000000) == ((int)floor(data->output[i][j] * 1000000.0 + 0.5))) {
					fprintf(file, "%d ", (int)data->output[i][j]);
				}
				else{
					fprintf(file, "%f ", data->output[i][j]);
				}
			}
#else
			fprintf(file, FANNPRINTF " ", data->output[i][j]);
#endif
		}
		fprintf(file, "\n");
	}

	return retval;
}

/*
 * Creates an empty set of training data
 */
FANN_EXTERNAL FannTrainData * FANN_API fann_create_train(uint num_data, uint num_input, uint num_output)
{
	fann_type * data_input, * data_output;
	uint i;
	FannTrainData * data = (FannTrainData*)malloc(sizeof(FannTrainData));
	if(data == NULL) {
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		return NULL;
	}
	fann_init_error_data((FannError*)data);
	data->num_data = num_data;
	data->num_input = num_input;
	data->num_output = num_output;
	data->input = (fann_type**)calloc(num_data, sizeof(fann_type *));
	if(data->input == NULL) {
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}
	data->output = (fann_type**)calloc(num_data, sizeof(fann_type *));
	if(data->output == NULL) {
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}
	data_input = (fann_type*)calloc(num_input * num_data, sizeof(fann_type));
	if(data_input == NULL) {
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}
	data_output = (fann_type*)calloc(num_output * num_data, sizeof(fann_type));
	if(data_output == NULL) {
		fann_error(NULL, FANN_E_CANT_ALLOCATE_MEM);
		fann_destroy_train(data);
		return NULL;
	}
	for(i = 0; i != num_data; i++) {
		data->input[i] = data_input;
		data_input += num_input;
		data->output[i] = data_output;
		data_output += num_output;
	}
	return data;
}

FANN_EXTERNAL FannTrainData * FANN_API fann_create_train_pointer_array(uint num_data,
    uint num_input, fann_type ** input, uint num_output, fann_type ** output)
{
	FannTrainData * data = fann_create_train(num_data, num_input, num_output);
	if(data) {
		for(uint i = 0; i < num_data; ++i) {
			memcpy(data->input[i], input[i], num_input*sizeof(fann_type));
			memcpy(data->output[i], output[i], num_output*sizeof(fann_type));
		}
	}
	return data;
}

FANN_EXTERNAL FannTrainData * FANN_API fann_create_train_array(uint num_data,
    uint num_input, fann_type * input, uint num_output, fann_type * output)
{
	FannTrainData * data = fann_create_train(num_data, num_input, num_output);
	if(data) {
		for(uint i = 0; i < num_data; ++i) {
			memcpy(data->input[i], &input[i*num_input], num_input*sizeof(fann_type));
			memcpy(data->output[i], &output[i*num_output], num_output*sizeof(fann_type));
		}
	}
	return data;
}

/*
 * Creates training data from a callback function.
 */
FANN_EXTERNAL FannTrainData * FANN_API fann_create_train_from_callback(uint num_data,
    uint num_input, uint num_output, void (FANN_API * user_function)(uint, uint, uint, fann_type *, fann_type *))
{
	FannTrainData * data = fann_create_train(num_data, num_input, num_output);
	if(data) {
		for(uint i = 0; i != num_data; i++) {
			(*user_function)(i, num_input, num_output, data->input[i], data->output[i]);
		}
	}
	return data;
}

FANN_EXTERNAL fann_type * FANN_API fann_get_train_input(FannTrainData * data, uint position)
{
	return (position >= data->num_data) ? NULL : data->input[position];
}

FANN_EXTERNAL fann_type * FANN_API fann_get_train_output(FannTrainData * data, uint position)
{
	return (position >= data->num_data) ? NULL : data->output[position];
}

/*
 * INTERNAL FUNCTION Reads training data from a file descriptor.
 */
FannTrainData * fann_read_train_from_fd(FILE * file, const char * filename)
{
	FannTrainData * data = 0;
	uint num_input, num_output, num_data, i, j;
	uint line = 1;
	if(fscanf(file, "%u %u %u\n", &num_data, &num_input, &num_output) != 3) {
		fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
	}
	else {
		line++;
		data = fann_create_train(num_data, num_input, num_output);
		if(data) {
			for(i = 0; i != num_data; i++) {
				for(j = 0; j != num_input; j++) {
					if(fscanf(file, FANNSCANF " ", &data->input[i][j]) != 1) {
						fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
						fann_destroy_train(data);
						return NULL;
					}
				}
				line++;
				for(j = 0; j != num_output; j++) {
					if(fscanf(file, FANNSCANF " ", &data->output[i][j]) != 1) {
						fann_error(NULL, FANN_E_CANT_READ_TD, filename, line);
						fann_destroy_train(data);
						return NULL;
					}
				}
				line++;
			}
		}
	}
	return data;
}
/*
 * INTERNAL FUNCTION returns 0 if the desired error is reached and -1 if it is not reached
 */
int fann_desired_error_reached(Fann * ann, float desired_error)
{
	switch(ann->train_stop_function) {
		case FANN_STOPFUNC_MSE:
		    if(fann_get_MSE(ann) <= desired_error)
			    return 0;
		    break;
		case FANN_STOPFUNC_BIT:
		    if(ann->num_bit_fail <= (uint)desired_error)
			    return 0;
		    break;
	}
	return -1;
}

#ifndef FIXEDFANN
/*
 * Scale data in input vector before feed it to ann based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_scale_input(Fann * ann, fann_type * input_vector)
{
	uint cur_neuron;
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
		return;
	}
	for(cur_neuron = 0; cur_neuron < ann->num_input; cur_neuron++)
		input_vector[ cur_neuron ] = ((input_vector[cur_neuron] - ann->scale_mean_in[cur_neuron]) /
			ann->scale_deviation_in[cur_neuron] - ((fann_type)-1.0) /* This is old_min */
		    )
		    * ann->scale_factor_in[cur_neuron]
		    + ann->scale_new_min_in[cur_neuron];
}

/*
 * Scale data in output vector before feed it to ann based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_scale_output(Fann * ann, fann_type * output_vector)
{
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
	}
	else {
		for(uint cur_neuron = 0; cur_neuron < ann->num_output; cur_neuron++) {
			output_vector[cur_neuron] = ((output_vector[cur_neuron] - ann->scale_mean_out[cur_neuron])
				/ ann->scale_deviation_out[cur_neuron] - ((fann_type)-1.0) /* This is old_min */
				) * ann->scale_factor_out[cur_neuron] + ann->scale_new_min_out[cur_neuron];
		}
	}
}

/*
 * Descale data in input vector after based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_descale_input(Fann * ann, fann_type * input_vector)
{
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
	}
	else {
		for(uint cur_neuron = 0; cur_neuron < ann->num_input; cur_neuron++) {
			input_vector[cur_neuron] = (
				(input_vector[cur_neuron] - ann->scale_new_min_in[cur_neuron])
				/ ann->scale_factor_in[cur_neuron]
				+ ((fann_type)-1.0)             /* This is old_min */
				)
				* ann->scale_deviation_in[cur_neuron]
				+ ann->scale_mean_in[cur_neuron];
		}
	}
}
/*
 * Descale data in output vector after get it from ann based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_descale_output(Fann * ann, fann_type * output_vector)
{
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
	}
	else {
		for(uint cur_neuron = 0; cur_neuron < ann->num_output; cur_neuron++) {
			output_vector[cur_neuron] = ((output_vector[cur_neuron] - ann->scale_new_min_out[cur_neuron])
				/ ann->scale_factor_out[cur_neuron]
				+ ((fann_type)-1.0)             /* This is old_min */
				) * ann->scale_deviation_out[cur_neuron] + ann->scale_mean_out[cur_neuron];
		}
	}
}
/*
 * Scale input and output data based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_scale_train(Fann * ann, FannTrainData * data)
{
	uint cur_sample;
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
		return;
	}
	/* Check that we have good training data. */
	if(fann_check_input_output_sizes(ann, data) == -1)
		return;
	for(cur_sample = 0; cur_sample < data->num_data; cur_sample++) {
		fann_scale_input(ann, data->input[ cur_sample ]);
		fann_scale_output(ann, data->output[ cur_sample ]);
	}
}
/*
 * Scale input and output data based on previously calculated parameters.
 */
FANN_EXTERNAL void FANN_API fann_descale_train(Fann * ann, FannTrainData * data)
{
	uint cur_sample;
	if(ann->scale_mean_in == NULL) {
		fann_error((FannError*)ann, FANN_E_SCALE_NOT_PRESENT);
		return;
	}
	// Check that we have good training data
	if(fann_check_input_output_sizes(ann, data) == -1)
		return;
	for(cur_sample = 0; cur_sample < data->num_data; cur_sample++) {
		fann_descale_input(ann, data->input[ cur_sample ]);
		fann_descale_output(ann, data->output[ cur_sample ]);
	}
}

#define SCALE_RESET(what, where, default_value)							      \
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++) \
		ann->what ## _ ## where[ cur_neuron ] = ( default_value );

#define SCALE_SET_PARAM(where)																		      \
        /* Calculate mean: sum(x)/length */																	\
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_mean_ ## where[ cur_neuron ] = 0.0f;													  \
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		for(cur_sample = 0; cur_sample < data->num_data; cur_sample++)								      \
			ann->scale_mean_ ## where[ cur_neuron ] += (float)data->where ## put[ cur_sample ][ cur_neuron ]; \
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_mean_ ## where[ cur_neuron ] /= (float)data->num_data;								  \
        /* Calculate deviation: sqrt(sum((x-mean)^2)/length) */												\
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_deviation_ ## where[ cur_neuron ] = 0.0f;												  \
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		for(cur_sample = 0; cur_sample < data->num_data; cur_sample++)								      \
			ann->scale_deviation_ ## where[ cur_neuron ] +=												  \
			    /* Another local variable in macro? Oh no! */										    \
			    (																						    \
			    (float)data->where ## put[ cur_sample ][ cur_neuron ]							      \
			    - ann->scale_mean_ ## where[ cur_neuron ]											      \
			    )																						    \
			    *																						    \
			    (																						    \
			    (float)data->where ## put[ cur_sample ][ cur_neuron ]							      \
			    - ann->scale_mean_ ## where[ cur_neuron ]											      \
			    );																						    \
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_deviation_ ## where[ cur_neuron ] =													  \
		    sqrtf(ann->scale_deviation_ ## where[ cur_neuron ] / (float)data->num_data);		    \
        /* Calculate factor: (new_max-new_min)/(old_max(1)-old_min(-1)) */									\
        /* Looks like we dont need whole array of factors? */												\
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_factor_ ## where[ cur_neuron ] =														  \
		    ( new_ ## where ## put_max - new_ ## where ## put_min )											    \
		    /																							    \
		    ( 1.0f - ( -1.0f ) );																	    \
        /* Copy new minimum. */																				\
        /* Looks like we dont need whole array of new minimums? */											\
	for(cur_neuron = 0; cur_neuron < ann->num_ ## where ## put; cur_neuron++)							  \
		ann->scale_new_min_ ## where[ cur_neuron ] = new_ ## where ## put_min;

FANN_EXTERNAL int FANN_API fann_set_input_scaling_params(Fann * ann,
    const FannTrainData * data, float new_input_min, float new_input_max)
{
	uint cur_neuron, cur_sample;
	/* Check that we have good training data. */
	/* No need for if( !params || !ann ) */
	if(data->num_input != ann->num_input || data->num_output != ann->num_output) {
		fann_error((FannError*)ann, FANN_E_TRAIN_DATA_MISMATCH);
		return -1;
	}
	if(ann->scale_mean_in == NULL)
		fann_allocate_scale(ann);
	if(ann->scale_mean_in == NULL)
		return -1;
	if(!data->num_data) {
		SCALE_RESET(scale_mean,                in,     0.0)
		SCALE_RESET(scale_deviation,   in,     1.0)
		SCALE_RESET(scale_new_min,             in,     -1.0)
		SCALE_RESET(scale_factor,              in,     1.0)
	}
	else {
		SCALE_SET_PARAM(in);
	}
	return 0;
}

FANN_EXTERNAL int FANN_API fann_set_output_scaling_params(Fann * ann,
    const FannTrainData * data, float new_output_min, float new_output_max)
{
	uint cur_neuron, cur_sample;
	/* Check that we have good training data. */
	/* No need for if( !params || !ann ) */
	if(data->num_input != ann->num_input || data->num_output != ann->num_output) {
		fann_error((FannError*)ann, FANN_E_TRAIN_DATA_MISMATCH);
		return -1;
	}
	if(ann->scale_mean_out == NULL)
		fann_allocate_scale(ann);
	if(ann->scale_mean_out == NULL)
		return -1;
	if(!data->num_data) {
		SCALE_RESET(scale_mean,                out,    0.0)
		SCALE_RESET(scale_deviation,   out,    1.0)
		SCALE_RESET(scale_new_min,             out,    -1.0)
		SCALE_RESET(scale_factor,              out,    1.0)
	}
	else {
		SCALE_SET_PARAM(out);
	}
	return 0;
}

/*
 * Calculate scaling parameters for future use based on training data.
 */
FANN_EXTERNAL int FANN_API fann_set_scaling_params(Fann * ann,
    const FannTrainData * data, float new_input_min,
    float new_input_max, float new_output_min, float new_output_max)
{
	if(fann_set_input_scaling_params(ann, data, new_input_min, new_input_max) == 0)
		return fann_set_output_scaling_params(ann, data, new_output_min, new_output_max);
	else
		return -1;
}
/*
 * Clears scaling parameters.
 */
FANN_EXTERNAL int FANN_API fann_clear_scaling_params(Fann * ann)
{
	uint cur_neuron;
	if(ann->scale_mean_out == NULL)
		fann_allocate_scale(ann);
	if(ann->scale_mean_out == NULL)
		return -1;
	SCALE_RESET(scale_mean,                in,     0.0)
	SCALE_RESET(scale_deviation,   in,     1.0)
	SCALE_RESET(scale_new_min,             in,     -1.0)
	SCALE_RESET(scale_factor,              in,     1.0)
	SCALE_RESET(scale_mean,                out,    0.0)
	SCALE_RESET(scale_deviation,   out,    1.0)
	SCALE_RESET(scale_new_min,             out,    -1.0)
	SCALE_RESET(scale_factor,              out,    1.0)
	return 0;
}

#endif

int fann_check_input_output_sizes(Fann * ann, FannTrainData * data)
{
	if(ann->num_input != data->num_input) {
		fann_error((FannError*)ann, FANN_E_INPUT_NO_MATCH, ann->num_input, data->num_input);
		return -1;
	}
	else if(ann->num_output != data->num_output) {
		fann_error((FannError*)ann, FANN_E_OUTPUT_NO_MATCH, ann->num_output, data->num_output);
		return -1;
	}
	else
		return 0;
}
