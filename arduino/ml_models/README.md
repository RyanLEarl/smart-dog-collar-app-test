Notes

1. colab_files contain files to use with google colab, detailed instructions are given in the Google Drive
2. tflite_models are files generated using the colab_files. These are not yet compatible with the arduino
3. byte_models are files generated using the tflite_models and are compatible with the arduino. These can be generated using "xxd -i file_name.tflite > file_name.cc"