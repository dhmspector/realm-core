/*************************************************************************
 *
 * REALM CONFIDENTIAL
 * __________________
 *
 *  [2011] - [2014] Realm Inc
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Realm Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Realm Incorporated
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Realm Incorporated.
 *
 **************************************************************************/
#ifndef REALM_ROW_HPP
#define REALM_ROW_HPP

#include <stdint.h>

#include <realm/util/type_traits.hpp>
#include <realm/mixed.hpp>
#include <realm/table_ref.hpp>
#include <realm/link_view_fwd.hpp>
#include <realm/handover_defs.hpp>

namespace realm {

template<class> class BasicRow;


/// This class is a "mixin" and contains the common set of functions for several
/// distinct row-like classes.
///
/// There is a direct and natural correspondance between the functions in this
/// class and functions in Table of the same name. For example:
///
///     table[i].get_int(j) == table.get_int(i,j)
///
/// The effect of calling most of the row accessor functions on a detached
/// accessor is unspecified and may lead to general corruption, and/or a
/// crash. The exceptions are is_attached(), detach(), get_table(), get_index(),
/// and the destructor. Note however, that get_index() will still return an
/// unspecified value for a deatched accessor.
///
/// When a row accessor is evaluated in a boolean context, it evaluates to true
/// if, and only if it is attached.
///
/// \tparam T A const or non-const table type (currently either `Table` or
/// `const Table`).
///
/// \tparam R A specific row accessor class (BasicRow or BasicRowExpr) providing
/// members `T* impl_get_table() const`, `std::size_t impl_get_row_ndx()
/// const`, and `void impl_detach()`. Neither are allowed to throw.
///
/// \sa Table
/// \sa BasicRow
template<class T, class R> class RowFuncs {
public:
    typedef T table_type;

    typedef BasicTableRef<const T> ConstTableRef;
    typedef BasicTableRef<T> TableRef; // Same as ConstTableRef if `T` is 'const'

    typedef typename util::CopyConst<T, LinkView>::type L;
    typedef util::bind_ptr<const L> ConstLinkViewRef;
    typedef util::bind_ptr<L> LinkViewRef; // Same as ConstLinkViewRef if `T` is 'const'

    int_fast64_t get_int(std::size_t col_ndx) const REALM_NOEXCEPT;
    bool get_bool(std::size_t col_ndx) const REALM_NOEXCEPT;
    float get_float(std::size_t col_ndx) const REALM_NOEXCEPT;
    double get_double(std::size_t col_ndx) const REALM_NOEXCEPT;
    StringData get_string(std::size_t col_ndx) const REALM_NOEXCEPT;
    BinaryData get_binary(std::size_t col_ndx) const REALM_NOEXCEPT;
    DateTime get_datetime(std::size_t col_ndx) const REALM_NOEXCEPT;
    ConstTableRef get_subtable(std::size_t col_ndx) const;
    TableRef get_subtable(std::size_t col_ndx);
    std::size_t get_subtable_size(std::size_t col_ndx) const REALM_NOEXCEPT;
    std::size_t get_link(std::size_t col_ndx) const REALM_NOEXCEPT;
    bool is_null_link(std::size_t col_ndx) const REALM_NOEXCEPT;
    bool is_null(std::size_t col_ndx) const REALM_NOEXCEPT;
    ConstLinkViewRef get_linklist(std::size_t col_ndx) const;
    LinkViewRef get_linklist(std::size_t col_ndx);
    bool linklist_is_empty(std::size_t col_ndx) const REALM_NOEXCEPT;
    std::size_t get_link_count(std::size_t col_ndx) const REALM_NOEXCEPT;
    Mixed get_mixed(std::size_t col_ndx) const REALM_NOEXCEPT;
    DataType get_mixed_type(std::size_t col_ndx) const REALM_NOEXCEPT;

    void set_int(std::size_t col_ndx, int_fast64_t value);
    void set_bool(std::size_t col_ndx, bool value);
    void set_float(std::size_t col_ndx, float value);
    void set_double(std::size_t col_ndx, double value);
    void set_string(std::size_t col_ndx, StringData value);
    void set_binary(std::size_t col_ndx, BinaryData value);
    void set_datetime(std::size_t col_ndx, DateTime value);
    void set_subtable(std::size_t col_ndx, const Table* value);
    void set_link(std::size_t col_ndx, std::size_t value);
    void nullify_link(std::size_t col_ndx);
    void set_mixed(std::size_t col_ndx, Mixed value);
    void set_mixed_subtable(std::size_t col_ndx, const Table* value);
    void set_null(std::size_t col_ndx);

    //@{
    /// Note that these operations will cause the row accessor to be detached.
    void remove();
    void move_last_over();
    //@}

    std::size_t get_backlink_count(const Table& src_table,
                                   std::size_t src_col_ndx) const REALM_NOEXCEPT;
    std::size_t get_backlink(const Table& src_table, std::size_t src_col_ndx,
                             std::size_t backlink_ndx) const REALM_NOEXCEPT;

    std::size_t get_column_count() const REALM_NOEXCEPT;
    DataType get_column_type(std::size_t col_ndx) const REALM_NOEXCEPT;
    StringData get_column_name(std::size_t col_ndx) const REALM_NOEXCEPT;
    std::size_t get_column_index(StringData name) const REALM_NOEXCEPT;

    /// Returns true if, and only if this accessor is currently attached to a
    /// row.
    ///
    /// A row accesor may get detached from the underlying row for various
    /// reasons (see below). When it does, it no longer refers to anything, and
    /// can no longer be used, except for calling is_attached(), detach(),
    /// get_table(), get_index(), and the destructor. The consequences of
    /// calling other methods on a detached row accessor are unspecified. There
    /// are a few Realm functions (Table::find_pkey_int()) that return a
    /// detached row accessor to indicate a 'null' result. In all other cases,
    /// however, row accessors obtained by calling functions in the Realm API
    /// are always in the 'attached' state immediately upon return from those
    /// functions.
    ///
    /// A row accessor becomes detached if the underlying row is removed, if the
    /// associated table accessor becomes detached, or if the detach() method is
    /// called. A row accessor does not become detached for any other reason.
    bool is_attached() const REALM_NOEXCEPT;

    /// Detach this accessor from the row it was attached to. This function has
    /// no effect if the accessor was already detached (idempotency).
    void detach() REALM_NOEXCEPT;

    /// The table containing the row to which this accessor is currently
    /// bound. For a detached accessor, the returned value is null.
    const table_type* get_table() const REALM_NOEXCEPT;
    table_type* get_table() REALM_NOEXCEPT;

    /// The index of the row to which this accessor is currently bound. For a
    /// detached accessor, the returned value is unspecified.
    std::size_t get_index() const REALM_NOEXCEPT;

#ifdef REALM_HAVE_CXX11_EXPLICIT_CONV_OPERATORS
    explicit operator bool() const REALM_NOEXCEPT;
#else
    typedef bool (RowFuncs::*unspecified_bool_type)() const;
    operator unspecified_bool_type() const REALM_NOEXCEPT;
#endif

private:
    const T* table() const REALM_NOEXCEPT;
    T* table() REALM_NOEXCEPT;
    std::size_t row_ndx() const REALM_NOEXCEPT;
};


/// This class is a special kind of row accessor. It differes from a real row
/// accessor (BasicRow) by having a trivial and fast copy constructor and
/// descructor. It is supposed to be used as the return type of functions such
/// as Table::operator[](), and then to be used as a basis for constructing a
/// real row accessor. Objects of this class are intended to only ever exist as
/// temporaries.
///
/// In contrast to a real row accessor (`BasicRow`), objects of this class do
/// not keep the parent table "alive", nor are they maintained (adjusted) across
/// row insertions and row removals like real row accessors are.
///
/// \sa BasicRow
template<class T> class BasicRowExpr:
        public RowFuncs<T, BasicRowExpr<T>> {
public:
    template<class U> BasicRowExpr(const BasicRowExpr<U>&) REALM_NOEXCEPT;

private:
    T* m_table; // nullptr if detached.
    std::size_t m_row_ndx; // Undefined if detached.

    BasicRowExpr(T*, std::size_t row_ndx) REALM_NOEXCEPT;

    T* impl_get_table() const REALM_NOEXCEPT;
    std::size_t impl_get_row_ndx() const REALM_NOEXCEPT;
    void impl_detach() REALM_NOEXCEPT;

    // Make impl_get_table(), impl_get_row_ndx(), and impl_detach() accessible
    // from RowFuncs.
    friend class RowFuncs<T, BasicRowExpr<T>>;

    // Make m_table and m_col_ndx accessible from BasicRowExpr(const
    // BasicRowExpr<U>&) for any U.
    template<class> friend class BasicRowExpr;

    // Make m_table and m_col_ndx accessible from
    // BasicRow::BaicRow(BasicRowExpr<U>) for any U.
    template<class> friend class BasicRow;

    // Make BasicRowExpr(T*, std::size_t) accessible from Table.
    friend class Table;
};

// fwd decl
class Group;

class RowBase {
protected:
    TableRef m_table; // nullptr if detached.
    std::size_t m_row_ndx; // Undefined if detached.

    void attach(Table*, std::size_t row_ndx) REALM_NOEXCEPT;
    void reattach(Table*, std::size_t row_ndx) REALM_NOEXCEPT;
    void impl_detach() REALM_NOEXCEPT;
    RowBase() { };

    typedef RowBase_Handover_patch Handover_patch;
    RowBase(const RowBase& source, Handover_patch& patch);
    void apply_patch(Handover_patch& patch, Group& group);
private:
    RowBase* m_prev = nullptr; // nullptr if first, undefined if detached.
    RowBase* m_next = nullptr; // nullptr if last, undefined if detached.

    // Table needs to be able to modify m_table and m_row_ndx.
    friend class Table;

};


/// An accessor class for table rows (a.k.a. a "row accessor").
///
/// For as long as it remains attached, a row accessor will keep the parent
/// table accessor alive. In case the lifetime of the parent table is not
/// managed by reference counting (such as when the table is an automatic
/// variable on the stack), the destruction of the table will cause all
/// remaining row accessors to be detached.
///
/// While attached, a row accessor is bound to a particular row of the parent
/// table. If that row is removed, the accesssor becomes detached. If rows are
/// inserted or removed before it (at lower row index), then the accessor is
/// automatically adjusted to account for the change in index of the row to
/// which the accessor is bound. In other words, a row accessor is bound to the
/// contents of a row, not to a row index. See also is_attached().
///
/// Row accessors are created and used as follows:
///
///     Row row       = table[7];  // 8th row of `table`
///     ConstRow crow = ctable[2]; // 3rd row of const `ctable`
///     Row first_row = table.front();
///     Row last_row  = table.back();
///
///     float v = row.get_float(1); // Get the float in the 2nd column
///     row.set_string(0, "foo");   // Update the string in the 1st column
///
///     Table* t = row.get_table();      // The parent table
///     std::size_t i = row.get_index(); // The current row index
///
/// \sa RowFuncs
template<class T> class BasicRow:
        private RowBase,
        public RowFuncs<T, BasicRow<T>> {
public:
    BasicRow() REALM_NOEXCEPT;

    template<class U> BasicRow(BasicRowExpr<U>) REALM_NOEXCEPT;
    BasicRow(const BasicRow<T>&) REALM_NOEXCEPT;
    template<class U> BasicRow(const BasicRow<U>&) REALM_NOEXCEPT;
    template<class U> BasicRow& operator=(BasicRowExpr<U>) REALM_NOEXCEPT;
    template<class U> BasicRow& operator=(BasicRow<U>) REALM_NOEXCEPT;
    BasicRow& operator=(const BasicRow<T>&) REALM_NOEXCEPT;

    ~BasicRow() REALM_NOEXCEPT;

private:
    T* impl_get_table() const REALM_NOEXCEPT;
    std::size_t impl_get_row_ndx() const REALM_NOEXCEPT;

    // Make impl_get_table(), impl_get_row_ndx(), and impl_detach() accessible
    // from RowFuncs.
    friend class RowFuncs<T, BasicRow<T>>;

    // Make m_table and m_col_ndx accessible from BasicRow(const BasicRow<U>&)
    // for any U.
    template<class> friend class BasicRow;

    std::unique_ptr<BasicRow<T>> clone_for_handover(std::unique_ptr<Handover_patch>& patch) const
    {
        patch.reset(new Handover_patch);
        std::unique_ptr<BasicRow<T>> retval(new BasicRow<T>(*this, *patch));
        return retval;
    }

    void apply_and_consume_patch(std::unique_ptr<Handover_patch>& patch, Group& group)
    {
        apply_patch(*patch, group);
        patch.reset();
    }

    void apply_patch(Handover_patch& patch, Group& group)
    {
        RowBase::apply_patch(patch, group);
    }

    BasicRow(const BasicRow<T>& source, Handover_patch& patch)
        : RowBase(source, patch)
    {
    }
    friend class SharedGroup;
};

typedef BasicRow<Table> Row;
typedef BasicRow<const Table> ConstRow;




// Implementation

template<class T, class R>
inline int_fast64_t RowFuncs<T,R>::get_int(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_int(col_ndx, row_ndx());
}

template<class T, class R>
inline bool RowFuncs<T,R>::get_bool(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_bool(col_ndx, row_ndx());
}

template<class T, class R>
inline float RowFuncs<T,R>::get_float(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_float(col_ndx, row_ndx());
}

template<class T, class R>
inline double RowFuncs<T,R>::get_double(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_double(col_ndx, row_ndx());
}

template<class T, class R>
inline StringData RowFuncs<T,R>::get_string(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_string(col_ndx, row_ndx());
}

template<class T, class R>
inline BinaryData RowFuncs<T,R>::get_binary(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_binary(col_ndx, row_ndx());
}

template<class T, class R>
inline DateTime RowFuncs<T,R>::get_datetime(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_datetime(col_ndx, row_ndx());
}

template<class T, class R>
inline typename RowFuncs<T,R>::ConstTableRef RowFuncs<T,R>::get_subtable(std::size_t col_ndx) const
{
    return table()->get_subtable(col_ndx, row_ndx()); // Throws
}

template<class T, class R>
inline typename RowFuncs<T,R>::TableRef RowFuncs<T,R>::get_subtable(std::size_t col_ndx)
{
    return table()->get_subtable(col_ndx, row_ndx()); // Throws
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_subtable_size(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_subtable_size(col_ndx, row_ndx());
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_link(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_link(col_ndx, row_ndx());
}

template<class T, class R>
inline bool RowFuncs<T,R>::is_null_link(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->is_null_link(col_ndx, row_ndx());
}

template<class T, class R>
inline bool RowFuncs<T,R>::is_null(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->is_null(col_ndx, row_ndx());
}

template<class T, class R> inline typename RowFuncs<T,R>::ConstLinkViewRef
RowFuncs<T,R>::get_linklist(std::size_t col_ndx) const
{
    return table()->get_linklist(col_ndx, row_ndx()); // Throws
}

template<class T, class R>
inline typename RowFuncs<T,R>::LinkViewRef RowFuncs<T,R>::get_linklist(std::size_t col_ndx)
{
    return table()->get_linklist(col_ndx, row_ndx()); // Throws
}

template<class T, class R>
inline bool RowFuncs<T,R>::linklist_is_empty(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->linklist_is_empty(col_ndx, row_ndx());
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_link_count(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_link_count(col_ndx, row_ndx());
}

template<class T, class R>
inline Mixed RowFuncs<T,R>::get_mixed(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_mixed(col_ndx, row_ndx());
}

template<class T, class R>
inline DataType RowFuncs<T,R>::get_mixed_type(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_mixed_type(col_ndx, row_ndx());
}

template<class T, class R>
inline void RowFuncs<T,R>::set_int(std::size_t col_ndx, int_fast64_t value)
{
    table()->set_int(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_bool(std::size_t col_ndx, bool value)
{
    table()->set_bool(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_float(std::size_t col_ndx, float value)
{
    table()->set_float(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_double(std::size_t col_ndx, double value)
{
    table()->set_double(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_string(std::size_t col_ndx, StringData value)
{
    table()->set_string(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_binary(std::size_t col_ndx, BinaryData value)
{
    table()->set_binary(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_datetime(std::size_t col_ndx, DateTime value)
{
    table()->set_datetime(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_subtable(std::size_t col_ndx, const Table* value)
{
    table()->set_subtable(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_link(std::size_t col_ndx, std::size_t value)
{
    table()->set_link(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::nullify_link(std::size_t col_ndx)
{
    table()->nullify_link(col_ndx, row_ndx()); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_mixed(std::size_t col_ndx, Mixed value)
{
    table()->set_mixed(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_mixed_subtable(std::size_t col_ndx, const Table* value)
{
    table()->set_mixed_subtable(col_ndx, row_ndx(), value); // Throws
}

template<class T, class R>
inline void RowFuncs<T,R>::set_null(std::size_t col_ndx)
{
    table()->set_null(col_ndx, row_ndx()); // Throws
}

template<class T, class R> inline void RowFuncs<T,R>::remove()
{
    table()->remove(row_ndx()); // Throws
}

template<class T, class R> inline void RowFuncs<T,R>::move_last_over()
{
    table()->move_last_over(row_ndx()); // Throws
}

template<class T, class R> inline std::size_t
RowFuncs<T,R>::get_backlink_count(const Table& src_table, std::size_t src_col_ndx) const
    REALM_NOEXCEPT
{
    return table()->get_backlink_count(row_ndx(), src_table, src_col_ndx);
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_backlink(const Table& src_table, std::size_t src_col_ndx,
                                               std::size_t backlink_ndx) const REALM_NOEXCEPT
{
    return table()->get_backlink(row_ndx(), src_table, src_col_ndx, backlink_ndx);
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_column_count() const REALM_NOEXCEPT
{
    return table()->get_column_count();
}

template<class T, class R>
inline DataType RowFuncs<T,R>::get_column_type(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_column_type(col_ndx);
}

template<class T, class R>
inline StringData RowFuncs<T,R>::get_column_name(std::size_t col_ndx) const REALM_NOEXCEPT
{
    return table()->get_column_name(col_ndx);
}

template<class T, class R>
inline std::size_t RowFuncs<T,R>::get_column_index(StringData name) const REALM_NOEXCEPT
{
    return table()->get_column_index(name);
}

template<class T, class R> inline bool RowFuncs<T,R>::is_attached() const REALM_NOEXCEPT
{
    return static_cast<const R*>(this)->impl_get_table();
}

template<class T, class R> inline void RowFuncs<T,R>::detach() REALM_NOEXCEPT
{
    static_cast<R*>(this)->impl_detach();
}

template<class T, class R> inline const T* RowFuncs<T,R>::get_table() const REALM_NOEXCEPT
{
    return table();
}

template<class T, class R> inline T* RowFuncs<T,R>::get_table() REALM_NOEXCEPT
{
    return table();
}

template<class T, class R> inline std::size_t RowFuncs<T,R>::get_index() const REALM_NOEXCEPT
{
    return row_ndx();
}

#ifdef REALM_HAVE_CXX11_EXPLICIT_CONV_OPERATORS

template<class T, class R> inline RowFuncs<T,R>::operator bool() const REALM_NOEXCEPT
{
    return is_attached();
}

#else // REALM_HAVE_CXX11_EXPLICIT_CONV_OPERATORS

template<class T, class R>
inline RowFuncs<T,R>::operator unspecified_bool_type() const REALM_NOEXCEPT
{
    return is_attached() ? &RowFuncs::is_attached : 0;
}

#endif // REALM_HAVE_CXX11_EXPLICIT_CONV_OPERATORS

template<class T, class R> inline const T* RowFuncs<T,R>::table() const REALM_NOEXCEPT
{
    return static_cast<const R*>(this)->impl_get_table();
}

template<class T, class R> inline T* RowFuncs<T,R>::table() REALM_NOEXCEPT
{
    return static_cast<R*>(this)->impl_get_table();
}

template<class T, class R> inline std::size_t RowFuncs<T,R>::row_ndx() const REALM_NOEXCEPT
{
    return static_cast<const R*>(this)->impl_get_row_ndx();
}


template<class T> template<class U>
inline BasicRowExpr<T>::BasicRowExpr(const BasicRowExpr<U>& expr) REALM_NOEXCEPT:
    m_table(expr.m_table),
    m_row_ndx(expr.m_row_ndx)
{
}

template<class T>
inline BasicRowExpr<T>::BasicRowExpr(T* table, std::size_t row_ndx) REALM_NOEXCEPT:
    m_table(table),
    m_row_ndx(row_ndx)
{
}

template<class T> inline T* BasicRowExpr<T>::impl_get_table() const REALM_NOEXCEPT
{
    return m_table;
}

template<class T> inline std::size_t BasicRowExpr<T>::impl_get_row_ndx() const REALM_NOEXCEPT
{
    return m_row_ndx;
}

template<class T> inline void BasicRowExpr<T>::impl_detach() REALM_NOEXCEPT
{
    m_table = nullptr;
}


template<class T> inline BasicRow<T>::BasicRow() REALM_NOEXCEPT
{
}

template<class T> inline BasicRow<T>::BasicRow(const BasicRow<T>& row) REALM_NOEXCEPT
    : RowBase(row)
{
    attach(const_cast<Table*>(row.m_table.get()), row.m_row_ndx);
}

template<class T> template<class U> inline BasicRow<T>::BasicRow(BasicRowExpr<U> expr) REALM_NOEXCEPT
{
    T* table = expr.m_table; // Check that pointer types are compatible
    attach(const_cast<Table*>(table), expr.m_row_ndx);
}

template<class T> template<class U> inline BasicRow<T>::BasicRow(const BasicRow<U>& row) REALM_NOEXCEPT
{
    T* table = row.m_table.get(); // Check that pointer types are compatible
    attach(const_cast<Table*>(table), row.m_row_ndx);
}

template<class T> template<class U>
inline BasicRow<T>& BasicRow<T>::operator=(BasicRowExpr<U> expr) REALM_NOEXCEPT
{
    T* table = expr.m_table; // Check that pointer types are compatible
    reattach(const_cast<Table*>(table), expr.m_row_ndx);
    return *this;
}

template<class T> template<class U>
inline BasicRow<T>& BasicRow<T>::operator=(BasicRow<U> row) REALM_NOEXCEPT
{
    T* table = row.m_table.get(); // Check that pointer types are compatible
    reattach(const_cast<Table*>(table), row.m_row_ndx);
    return *this;
}

template<class T>
inline BasicRow<T>& BasicRow<T>::operator=(const BasicRow<T>& row) REALM_NOEXCEPT
{
    reattach(const_cast<Table*>(row.m_table.get()), row.m_row_ndx);
    return *this;
}

template<class T> inline BasicRow<T>::~BasicRow() REALM_NOEXCEPT
{
    RowBase::impl_detach();
}

template<class T> inline T* BasicRow<T>::impl_get_table() const REALM_NOEXCEPT
{
    return m_table.get();
}

template<class T> inline std::size_t BasicRow<T>::impl_get_row_ndx() const REALM_NOEXCEPT
{
    return m_row_ndx;
}

} // namespace realm

#endif // REALM_ROW_HPP
