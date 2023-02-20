
/* Tools to manage credentials */


#include <Cred.h>


Cred::Cred() {
    NVS.begin();   // credentials data are stored on the long term memory
}


Cred::Cred(int n_cred, String* cred_list) {
    NVS.begin();   // credentials data are stored on the long term memory
    for(int i=0;i<n_cred;i++) {
        add_cred_to_list(cred_list[i]); 
    }
}


bool Cred::add_cred_to_list(String cred_name) {
    if(o_ncred < MAX_CRED-1) {                             // do we have space to add one more cred
        o_ncred++;
        o_cred_list[o_ncred-1] = cred_name;
        return false;
    } else {
        Serial.printf("ERROR: o_ncred >= MAX_CRED");
        return true;
    }
}


void Cred::clear_all_credentials_from_store() {            // Clear credentials from NVS memory 
  for(int i=0;i<o_ncred;i++) NVS.setString(o_cred_list[i], "");
}

bool Cred::request_cred_if_not_available() {
    bool ok = true;
    //Serial.printf("o_ncred:%d\n",o_ncred);
    for(int i=0;i<o_ncred;i++) {
      //Serial.printf("i:%d\n",i);
      if(NVS.getString(o_cred_list[i]) == "") {           // if not in NVS
        if(!request_cred_on_serial(o_cred_list[i])) {   // Request it on serial
            ok = false;                                 // if return is not true it had failed somewhere
        }
      }
    }
    return(ok);
}


bool Cred::request_cred_on_serial(String cred_name) {
    Serial.printf("\nThe following credential is missing: %s\n", cred_name);
    Serial.printf("Please enter %s:", cred_name);
    String secret = get_input_from_serial();
    bool ok       = NVS.setString(cred_name, secret);
    if(ok) Serial.printf("\n %s Updated with:%s\n", cred_name, secret);
    secret = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
    return(ok);
} 

/*
const char* Cred::get_cred_to_const_char(String cred_name) {
    Serial.printf("get_credential name: %s, from store\n",cred_name);
    String secret = NVS.getString(cred_name);
    //Serial.printf("secret :%s:\n", secret);
    //int len = secret.length() + 1;
    //char secret_ch[len];
    //Serial.printf("len %d:\n", len);
    //secret.toCharArray(secret_ch, len);
    //Serial.printf("secret_ch :%s:\n", secret_ch);
    //const char* secret_cch = secret_ch;
    static const char* secret_cch = secret.c_str();
    //Serial.printf("secret_cch :%s:\n", secret_cch);
    //for(int c=0;c<len;c++) {
    //  Serial.printf("C:%d\n",c);
    //  secret[c]    = '\n';  
    //  secret_ch[c] = '\n';
    //}
    return(secret_cch);
}
*/


String Cred::get_cred_to_String(String cred_name) {
    Serial.printf("get_credential name: %s, from store\n",cred_name);
    //static String secret = NVS.getString(cred_name);
    return(String( NVS.getString(cred_name)));
}

String Cred::get_input_from_serial() {             // Get user input from Serial
  char        buf[CRED_BUF_LEN];
  char        rc;
  byte        n      = 0;
  bool        newD   = false;

  while (Serial.available()) Serial.read();        // IMPORTANT: need to manualy flush input 
  while (newD == false && n < CRED_BUF_LEN) {
    if(Serial.available() > 0) {
      rc = Serial.read();
      if (rc != '\n' && int(rc) !=13) {            // ASCII 13 is caridge return
        buf[n] = rc;
        Serial.printf("%c",buf[n]);
        n++;
      } else {
        buf[n] = '\0';                             // terminate the string
        newD     = true;
        delay(20);                                 // wait a bit in case rubish arrives after 
        while (Serial.available()) Serial.read();  // IMPORTANT: need to manualy flush input 
      }
    }
  }
  Serial.println("\n:" + String(buf) + ":");
  return(String(buf));

}






