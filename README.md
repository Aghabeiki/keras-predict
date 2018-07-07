# keras-predict
A Nodejs Native addons to run Keras ML model.

 Run Keras ML model via [frugally-deep](https://github.com/Dobiasd/frugally-deep) as a native Nodejs Addons.
 
 usage: 
 * first [install](https://github.com/Dobiasd/frugally-deep/blob/master/INSTALL.md) frugally-deep.
 * compile your keras model :
 ```
python3 frugally-deep/keras_export/convert_model.py keras_model.h5 fdeep_model.json
 ```
 * path the size of your ML input and converted ML path to predict function.
 ```
const predict = require('keras-predict')
const path = require('path');
const mlModelPath = path.resolve(__dirname, "fdeep_model.json");
const testCase = [...];
predict.predict(120, mlModelPath, testCase, (err, predict) => {
        if (err) {
            console.error(err)
        } else {
            console.dir(predict.map(Math.round))
        }
    })
}
 ```
 
 * A working example avialable at example folder.

Amin Aghabeiki.