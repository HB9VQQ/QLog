#include "SearchFilterProxyModel.h"

SearchFilterProxyModel::SearchFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{}

void SearchFilterProxyModel::setSearchString(const QString &searchString)
{
    this->searchString = searchString;
    invalidateFilter();
}

void SearchFilterProxyModel::setSearchSkippedCols(const QVector<int> &columns)
{
    searchSkippedCols = columns;
    invalidateFilter();
}

bool SearchFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // full-text search
    for ( int col = 0; col < sourceModel()->columnCount(); ++col )
    {
        if (searchSkippedCols.contains(col) )
            continue;

        QModelIndex index = sourceModel()->index(source_row, col, source_parent);
        QString data = index.data(Qt::DisplayRole).toString();

        if ( data.contains(searchString, Qt::CaseInsensitive) )
            return true;
    }
    return false;
}

bool SearchFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QVariant leftData = sourceModel()->data(left, Qt::DisplayRole);
    QVariant rightData = sourceModel()->data(right, Qt::DisplayRole);

    bool leftOk, rightOk;
    int leftInt = leftData.toInt(&leftOk);
    int rightInt = rightData.toInt(&rightOk);

    // Both numeric → compare numerically
    if (leftOk && rightOk)
        return leftInt < rightInt;

    // Non-numeric values (e.g. "---") sort last (bottom)
    if (leftOk && !rightOk) return false;  // left is numeric, goes first
    if (!leftOk && rightOk) return true;   // right is numeric, goes first

    // Both non-numeric → fall back to string comparison
    return QSortFilterProxyModel::lessThan(left, right);
}
