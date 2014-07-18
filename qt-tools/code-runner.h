#ifndef CODERUNNER_H
#define CODERUNNER_H

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtWidgets/QComboBox>
#include <QtCore/QTimer>
#include <QtCore/QQueue>
#include <string>

#ifndef Q_MOC_RUN
#include <boost/function.hpp>
#endif //Q_MOC_RUN

#include "tools/helper.h"

//#ifdef WIN32
//to disable a visual studio compiler warning, that says that visual studio doesn't implement C++ exception specifications yet
//#pragma warning( disable : 4290 )
//#endif

namespace Tools
{
  struct CodeRunner;

  //! implement this interface to execute some code by CodeRunner via signals
	struct Code
	{
    Code() : _hasBeenInvokedAgain(false) {}
    virtual void exec(QVariantList /*params*/) {}
    virtual void exec() {}
    void runAgain(double seconds);//{ _cr->runAgain(seconds); }
    bool hasBeenInvokedAgain() const { return _hasBeenInvokedAgain; }
  private:
    bool _hasBeenInvokedAgain;
    CodeRunner* _cr;
    friend struct CodeRunner;
  };

  //! a wrapper to execute a piece of code, with the possibility of inline definition
	struct CodeRunner : public QObject
	{
    Q_OBJECT

		struct AutoDeleter
		{
      QList<CodeRunner*> crs;
      ~AutoDeleter(){ qDeleteAll(crs); }
      void add(CodeRunner* cr){ crs.append(cr); }
    };
    void autodelete(){ static AutoDeleter ad; ad.add(this); }

    Code* _code;
    bool deleteCodeObject;
    QVariantList _lastInvocationParams;
    QQueue<bool> _runAgainTimerSet;
  public:
    CodeRunner(Code* code, QObject* parent = 0)
		: QObject(parent), _code(code), deleteCodeObject(true)
		{
      if(!parent) autodelete();
      _code->_cr = this;
    }
    CodeRunner(Code& code, QObject* parent = 0)
		: QObject(parent), _code(&code), deleteCodeObject(false)
		{
      if(!parent) autodelete();
      _code->_cr = this;
    }
		virtual ~CodeRunner()
		{
      if(deleteCodeObject)
        delete _code;
    }

		void runAgain(double seconds)
		{
      QTimer::singleShot(int(seconds*1000), this, SLOT(executeCodeAgain()));
      //remember that the time has been set
      //even if runAgain has been called multiple times
      _runAgainTimerSet.enqueue(true);
    }

  public slots:
    //! run code without parameter
		void executeCode()
		{
      cleanup();
      _code->exec();
    }

    //! run code with bool parameter
		void executeCode(bool value)
		{
      cleanup();
      _code->exec(_lastInvocationParams = (QVariantList() << value));
    }

    //! run code with int parameter
		void executeCode(int value)
		{
      cleanup();
      _code->exec(_lastInvocationParams = (QVariantList() << value));
    }

    //! run code with double parameter
		void executeCode(double value)
		{
      cleanup();
      _code->exec(_lastInvocationParams = (QVariantList() << value));
    }

    //! slot which executes repeated calls with the same parameters
		void executeCodeAgain()
		{
			if(!_runAgainTimerSet.isEmpty())
			{
        _code->_hasBeenInvokedAgain = true;
        if(_lastInvocationParams.isEmpty())
          _code->exec();
        else
          _code->exec(_lastInvocationParams);// << true);

        _runAgainTimerSet.dequeue();
      }
    }

  private:
    //! upon a normal call, clear repeated calls which are now seen as obsolete
		void cleanup()
		{
      _runAgainTimerSet.clear();
      _code->_hasBeenInvokedAgain = false;
    }
  };

  //! convenience function to create a CodeRunner from a ptr to a code object
	inline CodeRunner* codeRunner(Code* c, QObject* parent = 0)
	{
    return new CodeRunner(c, parent);
  }

  /*! convenience function to create a CodeRunner from a reference to a code object
    * it is being assumed that a reference means that the caller handles
    * the object's life time, thus CodeRunner won't delete the Code object
    */
	inline CodeRunner* codeRunner(Code& c, QObject* parent = 0)
	{
    return new CodeRunner(c, parent);
  }

  inline void Code::runAgain(double seconds){ _cr->runAgain(seconds); }

}
#endif // CODERUNNER_H
