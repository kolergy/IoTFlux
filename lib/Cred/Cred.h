
#ifndef CRED
#define CRED

#include <Arduino.h>
#include <ArduinoNvs.h>

#define MAX_CRED 10
#define CRED_BUF_LEN 256

class Cred {

public:
    Cred(                                                 );
    Cred(                                   int, String*  );
    bool        add_cred_to_list(                String   );
    bool        request_cred_if_not_available(            );
    void        clear_all_credentials_from_store(         ); 
    String      get_cred_to_String(              String   );

private:
    String      o_cred_list[MAX_CRED] = {};               // "CRED_NAME"
    int         o_ncred               = 0;                // number of cred in list

    bool        request_cred_on_serial(          String   );
    String      get_input_from_serial(                    );

};

#endif //CRED






