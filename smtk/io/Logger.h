//=========================================================================
//  Copyright (c) Kitware, Inc.
//  All rights reserved.
//  See LICENSE.txt for details.
//
//  This software is distributed WITHOUT ANY WARRANTY; without even
//  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//  PURPOSE.  See the above copyright notice for more information.
//=========================================================================
#ifndef __smtk_io_Logger_h
#define __smtk_io_Logger_h
/*! \file */

#include "smtk/CoreExports.h"
#include "smtk/SystemConfig.h"
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/**\brief Write the expression \a x to \a logger as an error message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkErrorMacro(logger, x)                                                                  \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x;                                                                                       \
    logger.addRecord(smtk::io::Logger::ERROR, s1.str(), __FILE__, __LINE__);                       \
  } while (0)

/**\brief Write the expression \a x to \a logger as a warning message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkWarningMacro(logger, x)                                                                \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x;                                                                                       \
    logger.addRecord(smtk::io::Logger::WARNING, s1.str(), __FILE__, __LINE__);                     \
  } while (0)

/**\brief Write the expression \a x to \a logger as a debug message.
  *
  * Note that \a x may use the "<<" operator.
  */
#define smtkDebugMacro(logger, x)                                                                  \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x;                                                                                       \
    logger.addRecord(smtk::io::Logger::DEBUG, s1.str(), __FILE__, __LINE__);                       \
  } while (0)

/**\brief Write the expression \a x to \a logger as an informational message.
  *
  * Note that \a x may use the "<<" operator.
  *
  * Unlike other logging macros, this does not include  a
  * filename and line number in the record.
  */
#define smtkInfoMacro(logger, x)                                                                   \
  do                                                                                               \
  {                                                                                                \
    std::stringstream s1;                                                                          \
    s1 << x;                                                                                       \
    logger.addRecord(smtk::io::Logger::INFO, s1.str());                                            \
  } while (0)

namespace smtk
{
namespace io
{

/**\brief Log messages for later presentation to a user or a file.
 *
 * Logger has a singleton interface to a global logger, but is also
 * constructible as a non-singleton object.
 */
class SMTKCORE_EXPORT Logger
{
public:
  static Logger& instance();

  enum Severity
  {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
  };

  struct Record
  {
    Severity severity;
    std::string message;
    std::string fileName;
    unsigned int lineNumber;
    Record(Severity s, const std::string& m, const std::string& f = "", unsigned int l = 0)
      : severity(s)
      , message(m)
      , fileName(f)
      , lineNumber(l)
    {
    }
    Record()
      : severity(INFO)
      , lineNumber(0)
    {
    }
  };

  Logger()
    : m_hasErrors(false)
    , m_stream(nullptr)
    , m_ownStream(false)
  {
  }
  virtual ~Logger();
  std::size_t numberOfRecords() const { return this->m_records.size(); }

  bool hasErrors() const { return this->m_hasErrors; }

  void addRecord(
    Severity s, const std::string& m, const std::string& fname = "", unsigned int line = 0);

  const std::vector<Record>& records() const { return this->m_records; }
  const Record& record(std::size_t i) const { return this->m_records[i]; }

  std::string toString(std::size_t i, bool includeSourceLoc = false) const;
  std::string toString(std::size_t i, std::size_t j, bool includeSourceLoc = false) const;

  std::string toHTML(std::size_t i, std::size_t j, bool includeSourceLoc) const;

  // Convert all the messages into a single string
  std::string convertToString(bool includeSourceLoc = false) const;
  std::string convertToHTML(bool includeSourceLoc = false) const;
  void reset();

  void append(const Logger& l);

  static std::string severityAsString(Severity s);

  void setFlushToStream(std::ostream* output, bool ownFile, bool includePast);
  bool setFlushToFile(std::string filename, bool includePast);
  void setFlushToStdout(bool includePast);
  void setFlushToStderr(bool includePast);

  void setCallback(std::function<void()> fn);

protected:
  void flushRecordsToStream(std::size_t beginRec, std::size_t endRec);

  std::vector<Record> m_records;
  bool m_hasErrors;
  std::ostream* m_stream;
  bool m_ownStream;
  std::function<void()> m_callback;

private:
  static Logger m_instance;
};

template <typename J>
void to_json(J& json, const Logger::Record& rec)
{
  json = { { "severity", rec.severity }, { "message", rec.message }, { "file", rec.fileName },
    { "line", rec.lineNumber } };
}
}
}

#endif /* __smtk_io_Logger_h */
