/*=========================================================================

Copyright (c) 1998-2012 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved. No part of this software may be reproduced, distributed,
or modified, in any form or by any means, without permission in writing from
Kitware Inc.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF,
EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE IS PROVIDED ON AN
"AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE NO OBLIGATION TO
PROVIDE
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
=========================================================================*/
// .NAME AttributeReader.h -
// .SECTION Description Used to Read an Attribute Manager from a string or file
// .SECTION See Also

#ifndef __smtk_util_AttributeReader_h
#define __smtk_util_AttributeReader_h

#include "smtk/SMTKCoreExports.h"
#include <string>

namespace smtk
{
  namespace attribute
  {
    class Manager;
  }
  namespace util
  {
    class Logger;
    class SMTKCORE_EXPORT AttributeReader
    {
    public:
      // Returns true if there was a problem with reading the file
      bool read(smtk::attribute::Manager &manager,
                const std::string &filename,
                smtk::util::Logger &logger);
      bool readContents(smtk::attribute::Manager &manager,
                        const std::string &filecontents,
                        smtk::util::Logger &logger);

    protected:
    private:
    };
  }
}


#endif /* __smtk_util_AttributeReader_h */