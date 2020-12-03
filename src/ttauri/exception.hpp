

#pragma once

namespace tt {

/** Exception thrown during parsing on an error.
 * This exception is often thrown due to an error in the syntax
 * in both text and binary files.
 *
 * The `error_info`-`parse_location_tag` is often filled in for the
 * specific file name and location inside the file where the error
 * was.
 */
class parse_error : public std::runtime_error {};

/** Exception thrown during execution of a dynamic operation.
 * This exception is often thrown on operation between multiple polymorphic objects
 * which do not support the combined operation.
 *
 * For example a datum object may contain floating point number for which
 * a shift-right or shift-left would be an invalid operation.
 */
class operation_error : public std::runtime_error {};

}

