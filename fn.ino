 
int b[7] = {A15,A14,A13,A12,A11,A10,A9};
int b_rd[7];
int v[7] = {53,52,51,50,49,48,47};
int state[7];

int level [7][4] ={{5,10,35,40},{1,10,30,40},{30,40,50,60},{5,10,20,28},{40,50,60,70},{5,10,35,50},{5,10,35,50}};
int act [7];
int act_pre[7];
int send_number[7];
int send_number_pre[7];

int last = 0;
int p_position = 0;
int duration = 1500;
int speaker = 13;
int v_t[7];
int h_t[7];
int x = A0;
int x_rd=0;
int y = A1;
int y_rd=0;
int z = A2;
int z_rd=0;
// 41.20, 43.65, 46.25, 49.00, 51.91, 55.00, 58.27, 61.74, 65.41, 69.30, 73.42, 77.78, 82.41, 87.31, 92.50, 
// 98.00, 103.83, 110.00, 116.54, 123.47, 130.81, 138.59, 146.83, 155.56, 164.81, 174.61, 185.00, 196.00, 207.65, 220.00, 233.08, 246.94 
float strings[6][8] = {
  {164.81,  174.61, 185.00, 196.00, 207.65, 220.00, 233.08,   246.94},
  {123.47,  130.81, 138.59, 146.83, 155.56, 164.81, 174.61,   185.00},
  {98.00,   103.83, 110.00, 116.54, 123.47, 130.81, 138.59,   146.83},
  {73.42,   77.78,  82.41,  87.31,  92.50,  98.00,  103.83,   110.00},
  {55.00,   58.27,  61.74,  65.41,  69.30,  73.42,  77.78,    82.41},
  {41.20,   43.65,  46.25,  49.00,  51.91,  55.00,  58.27,    61.747},
};

int current_string = 0;
float tones[8] = {164.81,  174.61, 185.00, 196.00, 207.65, 220.00, 233.08,   246.94};
int h_act = 0;
int tone_t = 0;
bool l_muted = false;
bool r_muted = false;
int load_t = 0;
int l_stop_t = 0;
bool vibration_on = true;
bool z_load = 0;
int x_load = 0;
bool string_change = false;

void setup() {
  for(int i = 0; i<7; i++){
    pinMode(b[i], INPUT);
    pinMode(v[i], OUTPUT);
    b_rd[i] = 0;
    state[i] = 0;
    act[i] = 0;
    act_pre[i] = 0;
    v_t[i] = 0;
    h_t[i] = 0;
    send_number[i] = 0;
    send_number_pre[i] = 0;
    
  }
  pinMode(speaker, OUTPUT);
  pinMode(x,INPUT);
  pinMode(y,INPUT);
  pinMode(z,INPUT);
  Serial.begin(9600);
}

void vibration_on_check(){
  if(z_load == 0 && z_rd == 1){
    z_load = 1;
  }
  else if(z_load == 1 && z_rd == 0){
    z_load = 0;
    vibration_on = !vibration_on;
  }
}
void last_check(){
  last = 0;
  for(int i=0;i<7;i++){
    if(state[i] == 2){
      last = i+1;
    }
  }
}
//void speaker_debug(){
//  tone(13,1000);
//  delay(400);
//  noTone(13);
//}

bool timer_check(int a, int b, int l){
  if((a-b)>l){
    return true;
  }else{
    return false;
  }
}

bool hammer_check(int i){
  if(act[i] != 0){
    return (!timer_check(millis(),h_t[i],20));
  }
  return false;
}

void copy_act(){
  for (int i=0; i<7;i++){
    act_pre[i] = act[i];
  }
}

void vibration_timer_start(int i,int strength){
  if(vibration_on){
    analogWrite(v[i],strength);
    v_t[i] = millis();
  }
}

void vibration_timer_stop(int i){
  if( timer_check(millis(),v_t[i],150)){
    analogWrite(v[i],0);
  }
}

void state_change(int st, int ac, int i){
  state[i] = st;
  act[i] = ac;
}

void vibration_signal (int i){
  b_rd[i] = analogRead(b[i]);
  if(state[i] == 0){
    if(b_rd[i] > level[i][1]){
      l_stop_t = millis();
      state_change(1,1,i);
      vibration_timer_start(i,130);
      h_t[i] = millis();
    }
  }else if(state[i] == 1){
    if(b_rd[i] <level[i][0]){
      state_change(0,0,i);
      
    }else if(b_rd[i]>level[i][3]){
      if(hammer_check(i)){
        state_change(2,3,i);
        l_muted = l_muted_check();
        singing();
      }else{
        state_change(2,2,i);
        vibration_timer_start(i,150);
      }
    }
  }else if(state[i] == 2){
    if(b_rd[i] < level[i][0]){
      state_change(0,0,i);
    }else if(b_rd[i]<level[i][2]){
      l_stop_t = millis();
      state_change(1,0,i);
    }
  }
}

void send_signal(int i){
  if (act[i] != act_pre[i]){
    send_number_pre[i] = send_number[i];
    send_number[i] = (i+1)*10 + act[i];
    Serial.println(send_number[i]);
  }
}

void copy_signal(){
  for (int i=0; i<7;i++){
    send_number_pre[i] = send_number[i];
  }
}

void read_handle(){
  x_rd = 1023-analogRead(x);
  y_rd = 1023-analogRead(y);
  if(analogRead(z)==0){
    z_rd = 1;
  }else{
    z_rd = 0;
  }
}

void singing(){
  last_check();
  if (!l_muted && !r_muted){
    tone(speaker,tones[last]);
    tone_t = millis();
  }
}

void handle_check(){
    if(y_rd < 200 || y_rd > 900){
      load_t = millis();
    }
    if(h_act == 0 && y_rd < 300){
      r_muted = true;
      h_act = 1;
    }else if(h_act==1 && y_rd >400){
      r_muted = false;
      h_act = 0;
      if(!timer_check(millis(),load_t,15)){
        singing();
      }
    }else if(h_act==0 && y_rd > 700){
      r_muted = true;
      h_act = -1;
    }else if(h_act==-1 && y_rd <600){
      r_muted = false;
      h_act = 0;
      if(!timer_check(millis(),load_t,15)){
        singing();
      }
    }
}
void speaker_check(){
  if(tone_t != 0){
    int p = millis()-tone_t;
    if(p>duration){
      tone_t = 0;
      noTone(speaker);
    }
  }
}

bool l_muted_check(){
  bool r=false;
  for (int i=0; i<7; i++){
    if(state[i] == 1 && (timer_check(millis(),l_stop_t,20))){
      r = true;
    }
    if(state[i] == 2){
      r = false;
    }
  }
  return r;
}

bool singing_check(){
  if(l_muted || r_muted){
    tone_t = 0;
    noTone(speaker);
  }
  if(tone_t != 0){
    tone(speaker,tones[last]);
  }
}

void set_string_check(){
  if(string_change){
    noTone(speaker);
    string_change = false;
    for(int i=0;i<8; i++){
      tones[i] = strings[current_string][i];
    }
  } 
}

void change_string_check(){
  
  if(x_load == 0  && x_rd <20 && current_string != 5){
    x_load = 2;
  }else if(x_load == 2 && x_rd > 30 && current_string != 5){
    x_load = 0;
    current_string = current_string+1;
    string_change = true;
  }else if (x_load == 0 && x_rd > 1000 && current_string != 0){
    x_load = 1;
  }else if(x_load == 1 && x_rd < 900 &&x_rd >500 && current_string != 0){
    x_load = 0;
    current_string =current_string-1;
    string_change = true;
    
  }
}

void loop() {
  read_handle();
  copy_act();
  for(int i=0;i<7;i++){
    vibration_signal(i);
    send_signal(i);
    vibration_timer_stop (i);
  }
  vibration_on_check();
  last_check();
  handle_check();
  speaker_check();
  l_muted = l_muted_check();
  singing_check();
  change_string_check();
  set_string_check();

  //speaker_debug();
  //Serial.print(x_rd);
  //Serial.print("||");
  //Serial.println(y_rd);
  //Serial.println(millis());
  //Serial.print(r_muted);
  //Serial.println(l_muted);
  //Serial.println(last);
  //Serial.println(z_rd);
  //Serial.println(vibration_on);
  //Serial.println(current_string);
  //Serial.println(send_number[0]);
}
