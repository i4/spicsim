#ifndef FIX_VCD_H
#define FIX_VCD_H

#include <QString>

///
///  @brief  Fixes the signal initialization in the
///          VCD file stored under \p path_in and
///          writes the results to \p path_out
///
///  @param[in]  path_in   Path to the input file
///  @param[in]  path_out  Path to the result file
///
///  @returns  0 upon successful execution, -1 in case of an error
///
int fix_vcd( const QString& path_in, const QString& path_out );

#endif /* FIX_VCD_H */
