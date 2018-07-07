#include <napi.h>
#include <fdeep/fdeep.hpp>
inline void disable_loging(const std::string& str)
{
    // std::cout << str << std::flush;
}
class ErrorAsyncWorker : public Napi::AsyncWorker
{
  public:
    ErrorAsyncWorker(
        const Napi::Function &callback,
        Napi::Error error) : Napi::AsyncWorker(callback), error(error)
    {
    }

  protected:
    void Execute() override
    {
        // Do nothing...?
    }

    void OnOK() override
    {
        Napi::Env env = Env();

        Callback().MakeCallback(
            Receiver().Value(), {error.Value(),
                                 env.Undefined()});
    }

    void OnError(const Napi::Error &e) override
    {
        Napi::Env env = Env();

        Callback().MakeCallback(
            Receiver().Value(), {e.Value(),
                                 env.Undefined()});
    }

  private:
    Napi::Error error;
};

class PredicateAsyncWorker : public Napi::AsyncWorker
{
  public:
    PredicateAsyncWorker(const Napi::Function &callback,
                         int **testCases, int testCaseCount,
                         int footPrintSize, std::string mlModelPath)
        : Napi::AsyncWorker(callback), testCases(testCases), testCaseCount(testCaseCount),
          footPrintSize(footPrintSize), mlModelPath(mlModelPath)
    {
    }

  protected:
    void Execute() override
    {
        const auto model = fdeep::load_model(this->mlModelPath,true, disable_loging, static_cast<fdeep::float_type>(0.00001));
        this->results = new double[this->testCaseCount];
        fdeep::shape3 tensor3_shape(this->footPrintSize, 1, 1);
        for (int x = 0; x < this->testCaseCount; ++x)
        {
            fdeep::tensor3 t(tensor3_shape, 0.0f);
            for (int y = 0; y < this->footPrintSize; ++y)
            {
                t.set(y, 1, 1, this->testCases[x][y]);
            }
            const auto result = model.predict({t});
            fdeep::internal::tensor3_pos pos = fdeep::internal::tensor3_pos(0, 0, 0);
            this->results[x] = result[0].get(pos);
        }
    }

    void OnOK() override
    {
        Napi::Env env = Env();
        Napi::Array res = Napi::Array::New(env, this->testCaseCount);
        Napi::Number tmp;
        for (int i = 0; i < this->testCaseCount; i++)
        {
            res[i] = Napi::Number::New(env, this->results[i]);
        }
        Callback().MakeCallback(
            Receiver().Value(), {env.Null(), res});
    }

    void OnError(const Napi::Error &e) override
    {
        Napi::Env env = Env();
        Callback().MakeCallback(
            Receiver().Value(), {e.Value(),
                                 env.Undefined()});
    }

  private:
    int footPrintSize;
    std::string mlModelPath;
    int **testCases;
    int testCaseCount;
    double *results;
};

void PredictAsyncCallback(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();

    //
    // Account for known potential issues that MUST be handled by
    // synchronously throwing an `Error`
    //
    if (info.Length() < 4)
    {
        Napi::TypeError::New(env, "Invalid argument count").ThrowAsJavaScriptException();
        return;
    }
    if (!info[3].IsFunction())
    {
        Napi::TypeError::New(env, "Invalid argument types").ThrowAsJavaScriptException();
        return;
    }

    //
    // Handle all other potential issues asynchronously via the provided callback
    //

    Napi::Function cb = info[3].As<Napi::Function>();
    if (!info[0].IsNumber())
    {
        (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid argument types")))->Queue();
    }
    else if (!info[1].IsString())
    {
        (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid argument types")))->Queue();
    }
    else if (!info[2].IsArray())
    {
        (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid argument types")))->Queue();
    }
    else
    {
        // ml config
        // testCaseSize
        int footPrintSize = (int32_t)Napi::Number(env, info[0]);
        // ML model path
        std::string mlModelPath = std::string(Napi::String(env, info[1]));

        Napi::Array input = Napi::Array(env, info[2]);
        int testCaseCount = input.Length();
        int **testCases = new int *[testCaseCount];
        for (int i = 0; i < (int)testCaseCount; ++i)
        {
            testCases[i] = new int[footPrintSize];
            napi_value part;
            napi_get_element(env, info[2], i, &part);

            bool isarray;
            napi_is_array(env, part, &isarray);

            if (!isarray)
            {
                (new ErrorAsyncWorker(cb, Napi::TypeError::New(env, "Invalid test case.")))->Queue();
            }
            else
            {
                Napi::Array testCase = Napi::Array(env, part);
                if (testCase.Length() != footPrintSize)
                {
                    (new ErrorAsyncWorker(cb, Napi::TypeError::New(env,
                                                                   "Invalid test case. test case size should be as foot print size.")))
                        ->Queue();
                    return;
                }
                else
                {
                    for (int j = 0; j < footPrintSize; j++)
                    {
                        napi_value val;
                        int target;
                        napi_get_element(env, testCase, j, &val);
                        napi_get_value_int32(env, val, &target);
                        testCases[i][j] = target;
                    }
                }
            }
        }
        (new PredicateAsyncWorker(cb, testCases, testCaseCount, footPrintSize, mlModelPath))->Queue();
    }
    return;
}

// init nodejs module.
Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.Set(
        Napi::String::New(env, "predict"),
        Napi::Function::New(env, PredictAsyncCallback));
    return exports;
}

NODE_API_MODULE(predict, Init)
