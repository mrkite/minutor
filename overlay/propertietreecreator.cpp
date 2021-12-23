#include "propertietreecreator.h"

#include <QTreeWidgetItem>

PropertieTreeCreator::PropertieTreeCreator() {
//summary.insert("", "{id} ({Pos.[0]}, {Pos.[1]}, {Pos.[2]})");
  summary.insert("", "{id}");
  summary.insert("Pos", "({[0]}, {[1]}, {[2]})");
  summary.insert("Attributes[]", "{Name} = {Base}");
}

void PropertieTreeCreator::CreateTree(QTreeWidgetItem* node, const QVariant& v) {
  switch ((QMetaType::Type)v.type()) {
    case QMetaType::QVariantMap:
      ParseIterable(node, v.toMap());
      break;
    case QMetaType::QVariantHash:
      ParseIterable(node, v.toHash());
      break;
    case QMetaType::QVariantList:
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
    if ((QMetaType::Type)v.type() == QMetaType::Double) {
      return QString::number(v.toDouble(), 'f', 2);
    }
    return v.toString();
  } else if (subexpr.at(0) == '[') {
    int rightbracket = subexpr.indexOf(']');
    if (rightbracket > 0) {
      bool ok = false;
      int index = subexpr.mid(1, rightbracket-1).toInt(&ok);
      if (ok && (QMetaType::Type)v.type() == QMetaType::QVariantList) {
        return EvaluateSubExpression(subexpr.mid(rightbracket + 1),
                                     v.toList().at(index));
      }
    }
  } else {
    int dot = subexpr.indexOf('.');
    QString key = subexpr.mid(0, dot);
    if ((QMetaType::Type)v.type() == QMetaType::QVariantHash) {
      QHash<QString, QVariant> h = v.toHash();
      QHash<QString, QVariant>::const_iterator it = h.find(key);
      if (it != h.end())
        return EvaluateSubExpression(subexpr.mid(key.length() + 1), *it);
    } else if ((QMetaType::Type)v.type() == QMetaType::QVariantMap) {
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
  } else if ((QMetaType::Type)v.type() == QMetaType::QVariantList) {
    ret = QString("(%1 items)").arg(v.toList().size());
  } else if ((QMetaType::Type)v.type() == QMetaType::QVariantMap) {
    ret = GetSummary("", v);
  }
  return ret;
}
