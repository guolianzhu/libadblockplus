#ifndef ADBLOCKPLUS_JS_ENGINE_H
#define ADBLOCKPLUS_JS_ENGINE_H

#include <stdexcept>
#include <string>
#include <v8.h>
#include <AdblockPlus/JsValue.h>

namespace AdblockPlus
{
  class FileSystem;
  class WebRequest;
  class ErrorCallback;

  class JsError : public std::runtime_error
  {
  public:
    explicit JsError(const v8::Handle<v8::Value> exception,
        const v8::Handle<v8::Message> message);
  };

  class JsEngine
  {
    friend class JsValue;

  public:
    JsEngine(FileSystem* const fileReader,
             WebRequest* const webRequest,
             ErrorCallback* const errorCallback);
    JsValuePtr Evaluate(const std::string& source,
        const std::string& filename = "");
    void Load(const std::string& scriptPath);
    void Gc();
    JsValuePtr NewValue(const std::string& val);
    JsValuePtr NewValue(int64_t val);
    JsValuePtr NewValue(bool val);
    inline JsValuePtr NewValue(const char* val)
    {
      return NewValue(std::string(val));
    }
    inline JsValuePtr NewValue(int val)
    {
      return NewValue(static_cast<int64_t>(val));
    }

    class Context
    {
    public:
      Context(const JsEngine& jsEngine);
      virtual inline ~Context() {};

    private:
      const v8::Locker locker;
      const v8::HandleScope handleScope;
      const v8::Context::Scope contextScope;
    };

  private:
    const FileSystem* const fileSystem;
    v8::Isolate* isolate;
    v8::Persistent<v8::Context> context;
  };
}

#endif
