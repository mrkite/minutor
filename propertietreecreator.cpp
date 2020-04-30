#include "propertietreecreator.h"

#include <QTreeWidgetItem>

PropertieTreeCreator::PropertieTreeCreator() {
  summary.insert("", "{id} ({Pos.[0]}, {Pos.[1]}, {Pos.[2]})");
  summary.insert("Pos", "({[0]}, {[1]}, {[2]})");
  summary.insert("Attributes[]", "{Name} = {Base}");
}


template <class IterableT>
void PropertieTreeCreator::ParseIterable(QTreeWidgetItem* node, const IterableT& seq) {
  typename IterableT::const_iterator it, itEnd = seq.end();
  for (it = seq.begin(); it != itEnd; ++it) {
    QTreeWidgetItem* child = new QTreeWidgetItem();
    child->setData(0, Qt::DisplayRole, it.key());
    child->setData(1, Qt::DisplayRole, GetSummary(it.key(), it.value()));
    CreateTree(child, it.value());
    node->addChild(child);
  }
}


template <class IterableT>
void PropertieTreeCreator::ParseList(QTreeWidgetItem* node, const IterableT& seq) {
  typename IterableT::const_iterator it, itEnd = seq.end();
  int i = 0;
  // skip 1 sized arrays
  if (seq.size() == 0) {
    // empty
    if (node)
      node->setData(1, Qt::DisplayRole, "<empty>");
  } else if (seq.size() == 1) {
    CreateTree(node, seq.first());
    if (node)
      node->setData(1, Qt::DisplayRole, GetSummary("[0]", seq.first()));
  } else {
    for (it = seq.begin(); it != itEnd; ++it) {
      QTreeWidgetItem* child = new QTreeWidgetItem();
      QString key = QString("[%1]").arg(i++);
      child->setData(0, Qt::DisplayRole, key);
      child->setData(1, Qt::DisplayRole, GetSummary(key, *it));
      CreateTree(child, *it);

      node->addChild(child);
    }
  }
}


void PropertieTreeCreator::CreateTree(QTreeWidgetItem* node, const QVariant& v) {
  switch (v.type()) {
    case QVariant::Map:
      ParseIterable(node, v.toMap());
      break;
    case QVariant::Hash:
      ParseIterable(node, v.toHash());
      break;
    case QVariant::List:
      ParseList(node, v.toList());
      break;
    default:
      if (node)
        node->setData(1, Qt::DisplayRole, v.toString());
      break;
  }
}

QString EvaluateSubExpression(const QString& subexpr, const QVariant& v) {
  if (subexpr.size() == 0) {
    // limit the displayed decimal places
    if (v.type() == QVariant::Double) {
      return QString::number(v.toDouble(), 'f', 2);
    }
    return v.toString();
  } else if (subexpr.at(0) == '[') {
    int rightbracket = subexpr.indexOf(']');
    if (rightbracket > 0) {
      bool ok = false;
      int index = subexpr.mid(1, rightbracket-1).toInt(&ok);
      if (ok && v.type() == QVariant::List) {
        return EvaluateSubExpression(subexpr.mid(rightbracket + 1),
                                     v.toList().at(index));
      }
    }
  } else {
    int dot = subexpr.indexOf('.');
    QString key = subexpr.mid(0, dot);
    if (v.type() == QVariant::Hash) {
      QHash<QString, QVariant> h = v.toHash();
      QHash<QString, QVariant>::const_iterator it = h.find(key);
      if (it != h.end())
        return EvaluateSubExpression(subexpr.mid(key.length() + 1), *it);
    } else if (v.type() == QVariant::Map) {
      QMap<QString, QVariant> h = v.toMap();
      QMap<QString, QVariant>::const_iterator it = h.find(key);
      if (it != h.end())
        return EvaluateSubExpression(subexpr.mid(key.length() + 1), *it);
    }
  }

  return "";
}

QString PropertieTreeCreator::GetSummary(const QString& key, const QVariant& v) {
  QString ret;
  QMap<QString, QString>::const_iterator it = summary.find(key);
  if (it != summary.end()) {
    ret = *it;
    QRegularExpression re("(\\{.*?\\})");
    QRegularExpressionMatchIterator matches = re.globalMatch(*it);
    while (matches.hasNext()) {
      QRegularExpressionMatch m = matches.next();
      QString pattern = m.captured(0);
      QString evaluated =
          EvaluateSubExpression(pattern.mid(1, pattern.size() - 2), v);
      // if a lookup fails, then don't return anything
      if (evaluated.size() == 0) {
        ret = "";
        break;
      }
      ret.replace(pattern, evaluated);
    }
  } else if (v.type() == QVariant::List) {
    ret = QString("(%1 items)").arg(v.toList().size());
  } else if (v.type() == QVariant::Map) {
    ret = GetSummary("", v);
  }
  return ret;
}
