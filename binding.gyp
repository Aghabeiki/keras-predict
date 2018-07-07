 {
"targets":[ {
"target_name":"predicator", 
"sources":[ "KerasPredict.cpp" ], 
"include_dirs":[
        "<!@(node -p \"require('node-addon-api').include\")"
      ], 
"dependencies":[
        "<!@(node -p \"require('node-addon-api').gyp\")"
      ], 

#####
      #
      # Disable N - API C++wrapper classes to integrate C++/JS exception handling
      #
      #####
      "cflags!":[ "-fno-exceptions" ], 
"cflags_cc!":[ "-fno-exceptions"], 
# "cflags_cc":[ '-ggdb3', '-O0'],
"defines":[ "NAPI_DISABLE_CPP_EXCEPTIONS" ]
    }
]
}
