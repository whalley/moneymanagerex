// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2025 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *      Copyright: (c) 2022 Mark Whalley (mark@ipx.co.uk)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2025-05-08 09:16:56.228434.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include <vector>
#include <map>
#include <random>
#include <algorithm>
#include <functional>
#include <cwchar>
#include <wx/wxsqlite3.h>
#include <wx/intl.h>

#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

#include "html_template.h"
using namespace tmpl;

typedef wxLongLong int64;

class wxString;
enum OP { EQUAL = 0, GREATER, LESS, GREATER_OR_EQUAL, LESS_OR_EQUAL, NOT_EQUAL };

template<class V>
struct DB_Column
{
    V v_;
    OP op_;
    DB_Column(const V& v, OP op = EQUAL): v_(v), op_(op)
    {}
};

static int64 ticks_last_ = 0;
    
struct DB_Table
{
    DB_Table(): hit_(0), miss_(0), skip_(0) {};
    virtual ~DB_Table() {};
    wxString query_;
    size_t hit_, miss_, skip_;
    virtual wxString query() const { return this->query_; }
    virtual size_t num_columns() const = 0;
    virtual wxString name() const = 0;

    bool exists(wxSQLite3Database* db) const
    {
       return db->TableExists(this->name()); 
    }

    void drop(wxSQLite3Database* db) const
    {
        db->ExecuteUpdate("DROP TABLE IF EXISTS " + this->name());
    }

    static int64 newId()
    {
        // Get the current time in milliseconds as wxLongLong/int64
        int64 ticks = wxDateTime::UNow().GetValue();
        // Ensure uniqueness from last generated value
        if (ticks <= ticks_last_)
            ticks = ticks_last_ + 1;
        ticks_last_ = ticks;
        // Generate a random 3-digit number (0 to 999)
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(0, 999);
        int randomSuffix = dist(gen);
        // Combine ticks and randomSuffix
        return (ticks * 1000) + randomSuffix;
    }
};

template<typename Arg1>
void condition(wxString& out, bool /*op_and*/, const Arg1& arg1)
{
    out += Arg1::name();
    switch (arg1.op_)
    {
    case GREATER:           out += " > ? ";     break;
    case GREATER_OR_EQUAL:  out += " >= ? ";    break;
    case LESS:              out += " < ? ";     break;
    case LESS_OR_EQUAL:     out += " <= ? ";    break;
    case NOT_EQUAL:         out += " != ? ";    break;
    default:
        out += " = ? "; break;
    }
}

template<typename Arg1, typename... Args>
void condition(wxString& out, bool op_and, const Arg1& arg1, const Args&... args) 
{
    out += Arg1::name();
    switch (arg1.op_)
    {
    case GREATER:           out += " > ? ";     break;
    case GREATER_OR_EQUAL:  out += " >= ? ";    break;
    case LESS:              out += " < ? ";     break;
    case LESS_OR_EQUAL:     out += " <= ? ";    break;
    case NOT_EQUAL:         out += " != ? ";    break;
    default:
        out += " = ? "; break;
    }
    out += op_and? " AND " : " OR ";
    condition(out, op_and, args...);
}

template<typename Arg1>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1)
{
    stmt.Bind(index, arg1.v_);
}

template<typename Arg1, typename... Args>
void bind(wxSQLite3Statement& stmt, int index, const Arg1& arg1, const Args&... args)
{
    stmt.Bind(index, arg1.v_); 
    bind(stmt, index+1, args...);
}

template<typename TABLE, typename... Args>
const typename TABLE::Data_Set find_by(TABLE* table, wxSQLite3Database* db, bool op_and, const Args&... args)
{
    typename TABLE::Data_Set result;
    try
    {
        wxString query = table->query() + " WHERE ";
        condition(query, op_and, args...);
        wxSQLite3Statement stmt = db->PrepareStatement(query);
        bind(stmt, 1, args...);

        wxSQLite3ResultSet q = stmt.ExecuteQuery();

        while(q.NextRow())
        {
            typename TABLE::Data entity(q, table);
            result.push_back(std::move(entity));
        }

        q.Finalize();
    }
    catch(const wxSQLite3Exception &e) 
    { 
        wxLogError("%s: Exception %s", table->name().utf8_str(), e.GetMessage().utf8_str());
    }
 
    return result;
}

template<class DATA, typename Arg1>
bool match(const DATA* data, const Arg1& arg1)
{
    return data->match(arg1);
}

template<class DATA, typename Arg1, typename... Args>
bool match(const DATA* data, const Arg1& arg1, const Args&... args)
{
    return (data->match(arg1) && ... && data->match(args));
}

struct SorterByACCESSINFO
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ACCESSINFO) < (y.ACCESSINFO);
    }
};

struct SorterByACCOUNTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ACCOUNTID) < (y.ACCOUNTID);
    }
};

struct SorterByACCOUNTNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (std::wcscoll(x.ACCOUNTNAME.Lower().wc_str(),y.ACCOUNTNAME.Lower().wc_str()) < 0);  // Locale case-insensitive
    }
};

struct SorterByACCOUNTNUM
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ACCOUNTNUM) < (y.ACCOUNTNUM);
    }
};

struct SorterByACCOUNTTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ACCOUNTTYPE) < (y.ACCOUNTTYPE);
    }
};

struct SorterByACTIVE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ACTIVE) < (y.ACTIVE);
    }
};

struct SorterByAMOUNT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.AMOUNT) < (y.AMOUNT);
    }
};

struct SorterByASSETID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ASSETID) < (y.ASSETID);
    }
};

struct SorterByASSETNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ASSETNAME) < (y.ASSETNAME);
    }
};

struct SorterByASSETSTATUS
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ASSETSTATUS) < (y.ASSETSTATUS);
    }
};

struct SorterByASSETTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ASSETTYPE) < (y.ASSETTYPE);
    }
};

struct SorterByATTACHMENTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.ATTACHMENTID) < (y.ATTACHMENTID);
    }
};

struct SorterByBASECONVRATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.BASECONVRATE) < (y.BASECONVRATE);
    }
};

struct SorterByBDID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.BDID) < (y.BDID);
    }
};

struct SorterByBUDGETENTRYID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.BUDGETENTRYID) < (y.BUDGETENTRYID);
    }
};

struct SorterByBUDGETYEARID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.BUDGETYEARID) < (y.BUDGETYEARID);
    }
};

struct SorterByBUDGETYEARNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.BUDGETYEARNAME) < (y.BUDGETYEARNAME);
    }
};

struct SorterByCATEGID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CATEGID) < (y.CATEGID);
    }
};

struct SorterByCATEGNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (std::wcscoll(x.CATEGNAME.Lower().wc_str(),y.CATEGNAME.Lower().wc_str()) < 0);  // Locale case-insensitive
    }
};

struct SorterByCENT_NAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CENT_NAME) < (y.CENT_NAME);
    }
};

struct SorterByCHECKINGACCOUNTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CHECKINGACCOUNTID) < (y.CHECKINGACCOUNTID);
    }
};

struct SorterByCOLOR
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.COLOR) < (y.COLOR);
    }
};

struct SorterByCOMMISSION
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.COMMISSION) < (y.COMMISSION);
    }
};

struct SorterByCONTACTINFO
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CONTACTINFO) < (y.CONTACTINFO);
    }
};

struct SorterByCONTENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CONTENT) < (y.CONTENT);
    }
};

struct SorterByCREDITLIMIT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CREDITLIMIT) < (y.CREDITLIMIT);
    }
};

struct SorterByCURRDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRDATE) < (y.CURRDATE);
    }
};

struct SorterByCURRENCYID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRENCYID) < (y.CURRENCYID);
    }
};

struct SorterByCURRENCYNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return wxGetTranslation(x.CURRENCYNAME) < wxGetTranslation(y.CURRENCYNAME);
    }
};

struct SorterByCURRENCY_SYMBOL
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRENCY_SYMBOL) < (y.CURRENCY_SYMBOL);
    }
};

struct SorterByCURRENCY_TYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRENCY_TYPE) < (y.CURRENCY_TYPE);
    }
};

struct SorterByCURRENTPRICE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRENTPRICE) < (y.CURRENTPRICE);
    }
};

struct SorterByCURRHISTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRHISTID) < (y.CURRHISTID);
    }
};

struct SorterByCURRUPDTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRUPDTYPE) < (y.CURRUPDTYPE);
    }
};

struct SorterByCURRVALUE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.CURRVALUE) < (y.CURRVALUE);
    }
};

struct SorterByDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.DATE) < (y.DATE);
    }
};

struct SorterByDECIMAL_POINT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.DECIMAL_POINT) < (y.DECIMAL_POINT);
    }
};

struct SorterByDELETEDTIME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.DELETEDTIME) < (y.DELETEDTIME);
    }
};

struct SorterByDESCRIPTION
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.DESCRIPTION) < (y.DESCRIPTION);
    }
};

struct SorterByFAVORITEACCT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.FAVORITEACCT) < (y.FAVORITEACCT);
    }
};

struct SorterByFIELDATADID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.FIELDATADID) < (y.FIELDATADID);
    }
};

struct SorterByFIELDID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.FIELDID) < (y.FIELDID);
    }
};

struct SorterByFILENAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.FILENAME) < (y.FILENAME);
    }
};

struct SorterByFOLLOWUPID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.FOLLOWUPID) < (y.FOLLOWUPID);
    }
};

struct SorterByGROUPNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.GROUPNAME) < (y.GROUPNAME);
    }
};

struct SorterByGROUP_SEPARATOR
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.GROUP_SEPARATOR) < (y.GROUP_SEPARATOR);
    }
};

struct SorterByHELDAT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.HELDAT) < (y.HELDAT);
    }
};

struct SorterByHISTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.HISTID) < (y.HISTID);
    }
};

struct SorterByINFOID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INFOID) < (y.INFOID);
    }
};

struct SorterByINFONAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INFONAME) < (y.INFONAME);
    }
};

struct SorterByINFOVALUE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INFOVALUE) < (y.INFOVALUE);
    }
};

struct SorterByINITIALBAL
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INITIALBAL) < (y.INITIALBAL);
    }
};

struct SorterByINITIALDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INITIALDATE) < (y.INITIALDATE);
    }
};

struct SorterByINTERESTRATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.INTERESTRATE) < (y.INTERESTRATE);
    }
};

struct SorterByJSONCONTENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.JSONCONTENT) < (y.JSONCONTENT);
    }
};

struct SorterByLASTUPDATEDTIME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.LASTUPDATEDTIME) < (y.LASTUPDATEDTIME);
    }
};

struct SorterByLINKRECORDID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.LINKRECORDID) < (y.LINKRECORDID);
    }
};

struct SorterByLINKTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.LINKTYPE) < (y.LINKTYPE);
    }
};

struct SorterByLUACONTENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.LUACONTENT) < (y.LUACONTENT);
    }
};

struct SorterByMINIMUMBALANCE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.MINIMUMBALANCE) < (y.MINIMUMBALANCE);
    }
};

struct SorterByMINIMUMPAYMENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.MINIMUMPAYMENT) < (y.MINIMUMPAYMENT);
    }
};

struct SorterByNEXTOCCURRENCEDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.NEXTOCCURRENCEDATE) < (y.NEXTOCCURRENCEDATE);
    }
};

struct SorterByNOTES
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.NOTES) < (y.NOTES);
    }
};

struct SorterByNUMBER
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.NUMBER) < (y.NUMBER);
    }
};

struct SorterByNUMOCCURRENCES
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.NUMOCCURRENCES) < (y.NUMOCCURRENCES);
    }
};

struct SorterByNUMSHARES
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.NUMSHARES) < (y.NUMSHARES);
    }
};

struct SorterByPARENTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PARENTID) < (y.PARENTID);
    }
};

struct SorterByPATTERN
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PATTERN) < (y.PATTERN);
    }
};

struct SorterByPAYEEID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PAYEEID) < (y.PAYEEID);
    }
};

struct SorterByPAYEENAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (std::wcscoll(x.PAYEENAME.Lower().wc_str(),y.PAYEENAME.Lower().wc_str()) < 0);  // Locale case-insensitive
    }
};

struct SorterByPAYMENTDUEDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PAYMENTDUEDATE) < (y.PAYMENTDUEDATE);
    }
};

struct SorterByPERIOD
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PERIOD) < (y.PERIOD);
    }
};

struct SorterByPFX_SYMBOL
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PFX_SYMBOL) < (y.PFX_SYMBOL);
    }
};

struct SorterByPROPERTIES
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PROPERTIES) < (y.PROPERTIES);
    }
};

struct SorterByPURCHASEDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PURCHASEDATE) < (y.PURCHASEDATE);
    }
};

struct SorterByPURCHASEPRICE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.PURCHASEPRICE) < (y.PURCHASEPRICE);
    }
};

struct SorterByREFID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.REFID) < (y.REFID);
    }
};

struct SorterByREFTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.REFTYPE) < (y.REFTYPE);
    }
};

struct SorterByREPEATS
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.REPEATS) < (y.REPEATS);
    }
};

struct SorterByREPORTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.REPORTID) < (y.REPORTID);
    }
};

struct SorterByREPORTNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.REPORTNAME) < (y.REPORTNAME);
    }
};

struct SorterBySCALE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SCALE) < (y.SCALE);
    }
};

struct SorterBySETTINGID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SETTINGID) < (y.SETTINGID);
    }
};

struct SorterBySETTINGNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SETTINGNAME) < (y.SETTINGNAME);
    }
};

struct SorterBySETTINGVALUE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SETTINGVALUE) < (y.SETTINGVALUE);
    }
};

struct SorterBySFX_SYMBOL
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SFX_SYMBOL) < (y.SFX_SYMBOL);
    }
};

struct SorterBySHARECOMMISSION
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SHARECOMMISSION) < (y.SHARECOMMISSION);
    }
};

struct SorterBySHAREINFOID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SHAREINFOID) < (y.SHAREINFOID);
    }
};

struct SorterBySHARELOT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SHARELOT) < (y.SHARELOT);
    }
};

struct SorterBySHARENUMBER
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SHARENUMBER) < (y.SHARENUMBER);
    }
};

struct SorterBySHAREPRICE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SHAREPRICE) < (y.SHAREPRICE);
    }
};

struct SorterBySPLITTRANSAMOUNT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SPLITTRANSAMOUNT) < (y.SPLITTRANSAMOUNT);
    }
};

struct SorterBySPLITTRANSID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SPLITTRANSID) < (y.SPLITTRANSID);
    }
};

struct SorterBySQLCONTENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SQLCONTENT) < (y.SQLCONTENT);
    }
};

struct SorterBySTARTDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STARTDATE) < (y.STARTDATE);
    }
};

struct SorterBySTATEMENTDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STATEMENTDATE) < (y.STATEMENTDATE);
    }
};

struct SorterBySTATEMENTLOCKED
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STATEMENTLOCKED) < (y.STATEMENTLOCKED);
    }
};

struct SorterBySTATUS
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STATUS) < (y.STATUS);
    }
};

struct SorterBySTOCKID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STOCKID) < (y.STOCKID);
    }
};

struct SorterBySTOCKNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.STOCKNAME) < (y.STOCKNAME);
    }
};

struct SorterBySYMBOL
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.SYMBOL) < (y.SYMBOL);
    }
};

struct SorterByTAGID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TAGID) < (y.TAGID);
    }
};

struct SorterByTAGLINKID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TAGLINKID) < (y.TAGLINKID);
    }
};

struct SorterByTAGNAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TAGNAME) < (y.TAGNAME);
    }
};

struct SorterByTEMPLATECONTENT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TEMPLATECONTENT) < (y.TEMPLATECONTENT);
    }
};

struct SorterByTOACCOUNTID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TOACCOUNTID) < (y.TOACCOUNTID);
    }
};

struct SorterByTOTRANSAMOUNT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TOTRANSAMOUNT) < (y.TOTRANSAMOUNT);
    }
};

struct SorterByTRANSACTIONNUMBER
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSACTIONNUMBER) < (y.TRANSACTIONNUMBER);
    }
};

struct SorterByTRANSAMOUNT
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSAMOUNT) < (y.TRANSAMOUNT);
    }
};

struct SorterByTRANSCODE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSCODE) < (y.TRANSCODE);
    }
};

struct SorterByTRANSDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSDATE) < (y.TRANSDATE);
    }
};

struct SorterByTRANSID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSID) < (y.TRANSID);
    }
};

struct SorterByTRANSLINKID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TRANSLINKID) < (y.TRANSLINKID);
    }
};

struct SorterByTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.TYPE) < (y.TYPE);
    }
};

struct SorterByUNIT_NAME
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.UNIT_NAME) < (y.UNIT_NAME);
    }
};

struct SorterByUPDTYPE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.UPDTYPE) < (y.UPDTYPE);
    }
};

struct SorterByUSAGEDATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.USAGEDATE) < (y.USAGEDATE);
    }
};

struct SorterByUSAGEID
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.USAGEID) < (y.USAGEID);
    }
};

struct SorterByVALUE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.VALUE) < (y.VALUE);
    }
};

struct SorterByVALUECHANGE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.VALUECHANGE) < (y.VALUECHANGE);
    }
};

struct SorterByVALUECHANGEMODE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.VALUECHANGEMODE) < (y.VALUECHANGEMODE);
    }
};

struct SorterByVALUECHANGERATE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.VALUECHANGERATE) < (y.VALUECHANGERATE);
    }
};

struct SorterByWEBSITE
{ 
    template<class DATA>
    bool operator()(const DATA& x, const DATA& y)
    {
        return (x.WEBSITE) < (y.WEBSITE);
    }
};
