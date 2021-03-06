#include "accelerometer_handler.h"

#include "config.h"

#include "magic_wand_model_data.h"


#include "tensorflow/lite/c/common.h"

#include "tensorflow/lite/micro/kernels/micro_ops.h"

#include "tensorflow/lite/micro/micro_error_reporter.h"

#include "tensorflow/lite/micro/micro_interpreter.h"

#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"

#include "tensorflow/lite/schema/schema_generated.h"

#include "tensorflow/lite/version.h"

#include "mbed.h"

#include <cmath>

#include "DA7212.h"

#include "uLCD_4DGL.h"

#define bufferLength (32)

#define signalLength (1024)

DA7212 audio;

int idC = 0;

float signal[signalLength];

char serialInBuffer[bufferLength];

int serialCount = 0;

int16_t waveform[kAudioTxBufferSize];

EventQueue queue(32 * EVENTS_EVENT_SIZE);

// EventQueue queue_menu(32 * EVENTS_EVENT_SIZE);

Thread t;

Thread t1;

Thread t_load;

Thread t_DNN(osPriorityNormal, 120 * 1024);

Timer debounce;

Timer debounce_1;

Serial pc(USBTX, USBRX);

InterruptIn button(SW2);

InterruptIn button_MODE_SELECTION(SW3);

uLCD_4DGL uLCD(D1, D0, D2);

bool flag = 0;

bool stop = 1;

float volume;

int num_song = 0;

int i;

bool list = false;

int mode_num = 0;
//mode == 0 -> origin
//mode == 1 -> forward
//mode == 2 -> backward
//mode == 3 -> change song

void loadSignalHandler(void){

    queue.call(loadSignal);

}

void loadSignal(void){

  int load_song_num , load_melody;
  int i = 0;
  sending = 1;
  serialCount = 0;
  stop = 1;
  green_led = 0;

  while(i < signalLength){
    
    if(pc.readable()){

      serialInBuffer[serialCount] = pc.getc();

      serialCount++;

      if(serialCount == 3){

        load_song_num = (i/49);
        load_melody = (i%49);
        
        signal[i] = (int) atoi(serialInBuffer);
        
        if(a<3){
            
            song[load_song_num][load_melody]=signal[i];
            pc.printf("a=%d, b=%d, song=%d\r\n",load_song_num,load_melody,song[load_song_num][load_melody]);
        
        }
        
        else{
        
           noteLength[load_song_num-3][load_melody]=signal[i];
           pc.printf("a=%d, b=%d, noteLength=%d\r\n",load_song_num-3,load_melody,noteLength[load_song_num-3][load_melody]);
        
        }
        
        serialCount = 0;

        i++;

      }

    }

  }

  green_led = 1;  
  sending = 0;  
  stop = 1;
  num_song = 0;

}

void mode(){ //stop and continue

    if(debounce.read_ms()>1000){
        
        flag = !flag;
        debounce.reset();

    }

}


void selection(){  //selection is used to detect that whether you enter the menu or not

    if(debounce_1.read_ms()>1000){

        flag = false;
        
        list = true; 

    }

}

// int song[3][49] = {

//   {
//     261, 261, 392, 392, 440, 440, 392,

//     349, 349, 330, 330, 294, 294, 261,

//     392, 392, 349, 349, 330, 330, 294,

//     392, 392, 349, 349, 330, 330, 294,

//     261, 261, 392, 392, 440, 440, 392,

//     349, 349, 330, 330, 294, 294, 261,

//     0, 0, 0, 0, 0, 0, 0
//   },

//   {
//     392, 329, 329, 349, 293, 293, 261,

//     293, 329, 349, 392, 392, 392, 392,
    
//     329, 329, 349, 293, 293, 261, 329,
    
//     392, 392, 329,
    
//     293, 293, 293, 293, 293, 329, 349,
    
//     329, 329, 329, 329, 329, 349, 392,
    
//     392, 329, 329, 349, 293, 293, 261, 
    
//     329, 392, 392, 261      
//   },
//   {
//     261, 293, 329, 261, 261, 293, 329,

//     261, 329, 349, 392, 329, 349, 392,

//     392, 440, 392, 349, 329, 261 ,392,
    
//     440, 392, 349, 329, 261, 293, 196,

//     261, 293, 196, 261, 0, 0, 0,

//     0, 0, 0, 0, 0, 0, 0,

//     0, 0, 0, 0, 0, 0, 0,
//   }
  
// };

// int noteLength[3][49] = {
//   {
//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 2,

//     1, 1, 1, 1, 1, 1, 1
//   },
//   {
//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 3, 
  
//     1, 1, 1, 1, 1, 1, 1,
  
//     1, 1, 1, 1, 1, 1, 1, 
  
//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 3  
//   },
//   {
//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1,

//     1, 1, 1, 1, 1, 1, 1
//   }
  
// };


void playNote(int freq){

  for (int i = 0; i < kAudioTxBufferSize; i++)

  {

    waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1) * volume);

  }

  // the loop below will play the note for the duration of 1s

  for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j)

  {

    audio.spk.play(waveform, kAudioTxBufferSize);
    // idC = audio.spk.play(waveform, kAudioTxBufferSize);

  }

}



// Return the result of the last prediction

int PredictGesture(float* output) {

  // How many times the most recent gesture has been matched in a row

  static int continuous_count = 0;

  // The result of the last prediction

  static int last_predict = -1;


  // Find whichever output has a probability > 0.8 (they sum to 1)

  int this_predict = -1;

  for (int i = 0; i < label_num; i++) {

    if (output[i] > 0.8) this_predict = i;

  }


  // No gesture was detected above the threshold

  if (this_predict == -1) {

    continuous_count = 0;

    last_predict = label_num;

    return label_num;

  }


  if (last_predict == this_predict) {

    continuous_count += 1;

  } else {

    continuous_count = 0;

  }

  last_predict = this_predict;


  // If we haven't yet had enough consecutive matches for this gesture,

  // report a negative result

  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {

    return label_num;

  }

  // Otherwise, we've seen a positive result, so clear all our variables

  // and report it

  continuous_count = 0;

  last_predict = -1;


  return this_predict;

}


int DNN_main(int argc, char* argv[]) {


  // Create an area of memory to use for input, output, and intermediate arrays.

  // The size of this will depend on the model you're using, and may need to be

  // determined by experimentation.

  constexpr int kTensorArenaSize = 60 * 1024;

  uint8_t tensor_arena[kTensorArenaSize];


  // Whether we should clear the buffer next time we fetch data

  bool should_clear_buffer = false;

  bool got_data = false;


  // The gesture index of the prediction

  int gesture_index;


  // Set up logging.

  static tflite::MicroErrorReporter micro_error_reporter;

  tflite::ErrorReporter* error_reporter = &micro_error_reporter;


  // Map the model into a usable data structure. This doesn't involve any

  // copying or parsing, it's a very lightweight operation.

  const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);

  if (model->version() != TFLITE_SCHEMA_VERSION) {

    error_reporter->Report(

        "Model provided is schema version %d not equal "

        "to supported version %d.",

        model->version(), TFLITE_SCHEMA_VERSION);

    return -1;

  }


  // Pull in only the operation implementations we need.

  // This relies on a complete list of all the ops needed by this graph.

  // An easier approach is to just use the AllOpsResolver, but this will

  // incur some penalty in code space for op implementations that are not

  // needed by this graph.

  static tflite::MicroOpResolver<6> micro_op_resolver;

  micro_op_resolver.AddBuiltin(

      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,

      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());

  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,

                               tflite::ops::micro::Register_MAX_POOL_2D());

  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,

                               tflite::ops::micro::Register_CONV_2D());

  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,

                               tflite::ops::micro::Register_FULLY_CONNECTED());

  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,

                               tflite::ops::micro::Register_SOFTMAX());
                               
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,

                               tflite::ops::micro::Register_RESHAPE(),1);


  // Build an interpreter to run the model with

  static tflite::MicroInterpreter static_interpreter(

      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);

  tflite::MicroInterpreter* interpreter = &static_interpreter;


  // Allocate memory from the tensor_arena for the model's tensors

  interpreter->AllocateTensors();


  // Obtain pointer to the model's input tensor

  TfLiteTensor* model_input = interpreter->input(0);

  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||

      (model_input->dims->data[1] != config.seq_length) ||

      (model_input->dims->data[2] != kChannelNumber) ||

      (model_input->type != kTfLiteFloat32)) {

    error_reporter->Report("Bad input tensor parameters in model");

    return -1;

  }


  int input_length = model_input->bytes / sizeof(float);


  TfLiteStatus setup_status = SetupAccelerometer(error_reporter);

  if (setup_status != kTfLiteOk) {

    error_reporter->Report("Set up failed\n");

    return -1;

  }


  error_reporter->Report("Set up successful...\n");


  while (true) {


    // Attempt to read new data from the accelerometer

    got_data = ReadAccelerometer(error_reporter, model_input->data.f,

                                 input_length, should_clear_buffer);


    // If there was no new data,

    // don't try to clear the buffer again and wait until next time

    if (!got_data) {

      should_clear_buffer = false;

      continue;

    }


    // Run inference, and report any error

    TfLiteStatus invoke_status = interpreter->Invoke();

    if (invoke_status != kTfLiteOk) {

      error_reporter->Report("Invoke failed on index: %d\n", begin_index);

      continue;

    }


    // Analyze the results to obtain a prediction

    gesture_index = PredictGesture(interpreter->output(0)->data.f);


    // Clear the buffer next time we read data

    should_clear_buffer = gesture_index < label_num;


    // Produce an output

    if (gesture_index < label_num) {

        error_reporter->Report(config.output_message[gesture_index]);

        if(gesture_index == 0){ //ring

            mode_num = 1; //forward

            if(num_song == 2){
                num_song = 0;
            )

            else{
                num_song = num_song + 1;
            }

        } 

        else if(gesture_index == 1){ //slope

            mode_num = 2; //backward

            if(num_song == 0){
                num_song = 2
            }
            else{
                num_song = num_song -1;
            }

        }

        else if(gesture_index == 2){ //V

            list = false;
            mode_num = 0;
            flag = true;

        }

    }

  }

}

void show_info(){

  while(mode_num == 0){

    if(num_song == 0){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.printf("Twinkle Twinkle \nLittle Star\n");        
    
    }
    else if(num_song == 1){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.printf("Lightly Row\n");  

    }
    else if(num_song == 2){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.printf("Two Tigers\n"); 

    }

    if(flag){

        uLCD.locate(1,4);
        uLCD.printf("playing\n");
        uLCD.triangle(10, 100, 10, 110, 20, 105, 0X00FF00);
        uLCD.line(40, 100 , 40, 110, 0XFF0000);
        uLCD.line(45, 100 , 45, 110, 0XFF0000);

    }

    else{
            
        uLCD.locate(1,4);
        uLCD.printf("pause\n");
        uLCD.triangle(10, 100, 10, 110, 20, 105, 0XFF0000);
        uLCD.line(40, 100 , 40, 110, 0X00FF00);
        uLCD.line(45, 100 , 45, 110, 0X00FF00);

            
    }

  }

  while(list){
        
    pc.printf("menu work!");
            
    if(num_song == 0){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.color(RED);
        uLCD.printf("1.TwinkleTwinkle\n  LittleStar\n");
        uLCD.color(WHITE);
        uLCD.printf("2.LightlyRow\n");
        uLCD.color(WHITE);
        uLCD.printf("3.TwoTigers\n");
        uLCD.color(WHITE);
        uLCD.printf("Please use gestures to \nchoose song you want!");

    }
    else if(num_song == 1){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.color(WHITE);
        uLCD.printf("1.TwinkleTwinkle\n  LittleStar\n");
        uLCD.color(RED);
        uLCD.printf("2.LightlyRow\n");
        uLCD.color(WHITE);
        uLCD.printf("3.TwoTigers\n");
        uLCD.color(WHITE);
        uLCD.printf("Please use gestures to \nchoose song you want!");

    }
    else if(num_song == 2){

        uLCD.reset();
        uLCD.cls();
        uLCD.locate(1,2);
        uLCD.color(WHITE);
        uLCD.printf("1.TwinkleTwinkle\n  LittleStar\n");
        uLCD.color(WHITE);
        uLCD.printf("2.LightlyRow\n");
        uLCD.color(RED);
        uLCD.printf("3.TwoTigers\n");
        uLCD.color(WHITE);
        uLCD.printf("Please use gestures to \nchoose song you want!");

    }

  }

}

int main(){

    t_load.start(queue.event(loadSignalHandler));
    t.start(callback(&queue, &EventQueue::dispatch_forever));
    t1.start(show_info);
    t_DNN.start(DNN_main);

    debounce.start();

    debounce_1.start();

    button.rise(&mode);

    button_MODE_SELECTION.rise(&selection);

    int length;

    for(i = 0; i < 49; i++){
    
        pc.printf("num_song == %d",num_song);

        length = noteLength[num_song][i];

        while(length--){

            if(flag){
                volume = 0.2;
                pc.printf("flag == 1");
                queue.call(playNote, song[num_song][i]);
            }
            else{
                volume = 0;
                pc.printf("flag == 0");
                // queue.event(stopPlayNote);
                queue.call(playNote, song[num_song][i]);
                // length++;
                if(i>0) i--;
                else i = 0 ;  
            }

            if(length <= 1) wait(1.0);

        }

    }    

}
