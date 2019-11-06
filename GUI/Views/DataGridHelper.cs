using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using Microsoft.Toolkit.Uwp.UI.Controls;

namespace GUI.Views
{
    /// <summary>
    /// Extension methods used with the DataGrid control to support sorting.
    /// </summary>
    public static class DataGridHelper
    {
        public static void Sort(this DataGrid dataGrid, DataGridColumn columnToSort, Action<string, bool> sort)
        {
            var lastSortedColumn = dataGrid.Columns.Where(column =>
                column.SortDirection.HasValue).FirstOrDefault();
            bool isSortColumnDifferentThanLast = columnToSort != lastSortedColumn;
            bool isAscending = isSortColumnDifferentThanLast ||
                columnToSort.SortDirection == DataGridSortDirection.Descending;

            columnToSort.SortDirection = isAscending ?
                DataGridSortDirection.Ascending : DataGridSortDirection.Descending;
            if (isSortColumnDifferentThanLast && lastSortedColumn != null)
            {
                lastSortedColumn.SortDirection = null;
            }

            var propertyName = columnToSort.Tag as string ?? columnToSort.Header.ToString();
            sort(propertyName, isAscending);
        }


        public static void Sort<T>(this ObservableCollection<T> collection, string propertyName, bool isAscending)
        {
            object sortFunc(T obj) => obj.GetType().GetProperty(propertyName).GetValue(obj);
            List<T> sortedCollection = isAscending ?
                collection.OrderBy(sortFunc).ToList() :
                collection.OrderByDescending(sortFunc).ToList();
            collection.Clear();
            foreach (var obj in sortedCollection)
            {
                collection.Add(obj);
            }
        }
    }
}
