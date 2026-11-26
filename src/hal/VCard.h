#pragma once

#include "hal/PhoneHal.h"

#include <QByteArray>
#include <QVector>

/**
 * Leitura de vCards (2.1 e 3.0) vindos do celular pelo PBAP.
 *
 * Os celulares mandam de tudo: linhas dobradas, QUOTED-PRINTABLE com acentos em UTF-8
 * (vCard 2.1 do Android), vários TEL por contato, só N sem FN... Aqui fica só o que a
 * central usa: nome + números (agenda) e tipo/data/hora (histórico de chamadas).
 */
namespace VCard {

/** Agenda ("pb"): um Contact por número. Contatos sem número são ignorados. */
QVector<Contact> parseContacts(const QByteArray &data);

/**
 * Histórico ("cch", "ich", "och", "mch"). O tipo vem de X-IRMC-CALL-DATETIME
 * (RECEIVED/DIALED/MISSED); se faltar, usa [defaultKind] ("in", "out" ou "missed").
 * Ordenado do mais recente para o mais antigo.
 */
QVector<RecentCall> parseHistory(const QByteArray &data, const QString &defaultKind = QString());

} // namespace VCard
