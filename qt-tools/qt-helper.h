#ifndef QT_HELPER_H_
#define QT_HELPER_H_

#include <QtCore/QObject>
#include <QtCore/QVariantList>
#include <QtWidgets/QComboBox>
#include <QtCore/QTimer>
#include <QtCore/QQueue>
#include <string>

#include <boost/function.hpp>

#include "tools/helper.h"
#include "code-runner.h"

//#ifdef WIN32
//to disable a visual studio compiler warning, that says that visual studio doesn't implement C++ exception specifications yet
//#pragma warning( disable : 4290 )
//#endif

namespace Tools
{
	std::function<std::string (double)>
	gridValueTransformFunction4datasetName(const std::string& datasetName);

	//----------------------------------------------------------------------------

	//! atIndex == -1 means currentIndex
	/*!
	 * get typed value back from combobox
	 * @param cb the combobox
	 * @param atIndex the optional index of the requested data value
	 * @param def the optional default value if no value is being found
	 * @return pointer value stored as data in combobox entry
	 */
	template<class T>
	T dataValue(const QComboBox* cb, int atIndex = -1, T def = T())
	{
		if(atIndex == -1)
			atIndex = cb->currentIndex();
		if(atIndex < 0)
			return def;
    return qvariant_cast<T>(cb->itemData(atIndex));
	}

	//! atIndex == -1 means currentIndex
	/*!
	 * get typed value back from combobox
	 * @param cb the combobox
	 * @param atIndex the optional index of the requested data value
	 * @param def the optional default value if no value is being found
	 * @return pointer value stored as data in combobox entry
	 */
	//	template<class T>
	//  T* dataValue(const QComboBox* cb, int atIndex = -1, T* def = NULL){
	//		if(atIndex == -1) atIndex = cb->currentIndex();
	//    T* t = qVariantValue<T*>(cb->itemData(atIndex));//.value<T*>();
	//    //T* t = cb->itemData(atIndex).value<T*>();
	//		return t ? t : def;
	//	}

	//! marker for NULL value
	struct IsNULL {};

	/*!
	 * same as dataValue, but uses exceptions to be able
	 * to chain many calls to dataValueE which can possibly fail
	 * @param cb the combobox
	 * @param atIndex the optional index
	 * @return pointer value stored as data in combobox entry
	 */
	template<class T>
	T dataValueE(QComboBox* cb, int atIndex = -1) throw (IsNULL)
	{
		T res = dataValue<T>(cb, atIndex);
		if(!res)
			throw IsNULL();
		return res;
	}

	//! set the combobox to the given datavalue
	template<class T>
	void select(QComboBox* cb, T t)
	{
		cb->setCurrentIndex(cb->findData(QVariant::fromValue(t)));
	}

	/*!
	 * functor filling a combobox with all values from the given collection
	 * - collection entries have to have a name attribute of std::string type
	 * - entries also either have to be convertible to a QVariant
	 * or be a std::pair whose first or second entry is convertible to a QVariant
	 */
	template<class Collection, template<class> class GetValue = Id>
	struct FillCB
	{
		/*!
		 * the operator filling the combobox
		 * @param cb the combobox to be filled (will be emptied first)
		 * @param col the collection to be used
		 * @param blockSignals optionally block signals from this combobox during inserts
		 * @param setIndex optionally set the index finally
		 * @param index optionally set this index if the index should be set at all
		 */
		void operator()(QComboBox* cb, Collection& col, bool blockSignals = false,
										bool setIndex = true, int index = -1)
		{
			if(blockSignals)
				cb->blockSignals(true);
			cb->clear();
			typedef typename Collection::value_type CVT;
			typedef typename Collection::const_iterator CI;
			for(CI ci = col.begin(); ci != col.end(); ci++)
			{
				cb->addItem(QString::fromUtf8(GetValue<CVT>()(*ci)->name.c_str()),
										QVariant::fromValue(GetValue<CVT>()(*ci)));
			}
			if(setIndex)
				cb->setCurrentIndex(index);
			if(blockSignals)
				cb->blockSignals(false);
		}
	};

	//! convenience function for the FillCB functor with Collection type inference
	template<class Collection>
	void fillCB(QComboBox* cb, Collection& col, bool blockSignals = false,
							bool setIndex = true, int index = -1)
	{
		FillCB<Collection>()(cb, col, blockSignals, setIndex, index);
	}

	//----------------------------------------------------------------------------

	struct JaNein {
		JaNein() : value(false) {}
		JaNein(bool v) : value(v) {}
		bool value;
		bool operator==(const JaNein& o) const { return value == o.value; }
		std::string toString() const;
		static JaNein ja(){ return JaNein(JA); }
		static JaNein nein(){ return JaNein(NEIN); }
		static const bool JA;
		static const bool NEIN;
	};

	//----------------------------------------------------------------------------

	class GermanBoolEditor : public QComboBox
	{
		Q_OBJECT
		Q_PROPERTY(Tools::JaNein jaNein READ jaNein WRITE setJaNein USER true)
	public:
		GermanBoolEditor(QWidget* parent = 0);
		JaNein jaNein() const;
		void setJaNein(JaNein jn);
	};

	//----------------------------------------------------------------------------

	QPair<QVector<double>, QVector<double> >
	createMinMaxPolygon(const QVector<double>& xs,
											const QVector<double>& mins,
											const QVector<double>& maxs);

	QPair<QVector<double>, QVector<double> >
	createStandardDeviationPolygon(const QVector<double>& xs,
																 const QVector<double>& ys,
																 const QVector<double>& sigmas);

}

Q_DECLARE_METATYPE(Tools::JaNein)

#endif
