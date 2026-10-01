'use client';

import { useEffect, useState } from 'react';
import { cardApi, type Card } from '@/lib/cards';

export default function CardManager() {
    const [cards, setCards] = useState<Card[]>([]);
    const [draft, setDraft] = useState<Card[]>([]);
    const [editingUid, setEditingUid] = useState<string | null>(null);
    const [loading, setLoading] = useState(true);
    const [saving, setSaving] = useState(false);
    const [error, setError] = useState<string | null>(null);

    useEffect(() => {
        let cancelled = false;

        async function load() {
            try {
                const data = await cardApi.list();
                if (cancelled) {
                    return;
                }
                setCards(data);
                setDraft(data);
            } catch (err) {
                if (cancelled) {
                    return;
                }
                setError(err instanceof Error ? err.message : 'Failed to load cards');
            } finally {
                if (!cancelled) {
                    setLoading(false);
                }
            }
        }

        void load();

        return () => {
            cancelled = true;
        };
    }, []);

    useEffect(() => {
        const source = new EventSource(cardApi.eventsUrl());

        source.onmessage = (event) => {
            const data = JSON.parse(event.data) as { cards: Card[] };
            setCards(data.cards);
            setDraft((prev) => {
                const prevByUid = new Map(prev.map((c) => [c.uid, c]));
                return data.cards.map((c) => prevByUid.get(c.uid) ?? c);
            });
        };

        return () => {
            source.close();
        };
    }, []);

    const dirty = JSON.stringify(draft) !== JSON.stringify(cards);

    function startEdit(card: Card) {
        setEditingUid(card.uid);
    }

    function updateEmail(uid: string, email: string) {
        setDraft((prev) => prev.map((c) => (c.uid === uid ? { ...c, email } : c)));
    }

    function finishEdit() {
        setEditingUid(null);
    }

    async function save() {
        setSaving(true);
        setError(null);
        try {
            const saved = await cardApi.save(draft);
            setCards(saved);
            setDraft(saved);
            setEditingUid(null);
        } catch (err) {
            setError(err instanceof Error ? err.message : 'Failed to save cards');
        } finally {
            setSaving(false);
        }
    }

    function cancel() {
        setDraft(cards);
        setEditingUid(null);
        setError(null);
    }

    if (loading) {
        return <p className="text-zinc-500">Đang tải danh sách thẻ...</p>;
    }

    return (
        <div className="w-full max-w-2xl">
            <div className="mb-4 flex items-center justify-between">
                <h1 className="text-xl font-semibold">Quản lý thẻ RFID</h1>
                <div className="flex gap-2">
                    <button
                        type="button"
                        onClick={save}
                        disabled={!dirty || saving}
                        className="rounded-md bg-foreground px-4 py-2 text-sm font-medium text-background transition-colors hover:opacity-90 disabled:cursor-not-allowed disabled:opacity-40"
                    >
                        {saving ? 'Đang lưu...' : 'Lưu'}
                    </button>
                    <button
                        type="button"
                        onClick={cancel}
                        disabled={!dirty || saving}
                        className="rounded-md border border-black/[.12] px-4 py-2 text-sm font-medium transition-colors hover:bg-black/[.04] disabled:cursor-not-allowed disabled:opacity-40"
                    >
                        Hủy
                    </button>
                </div>
            </div>

            {error && (
                <p className="mb-4 rounded-md bg-red-50 px-3 py-2 text-sm text-red-600">
                    {error}
                </p>
            )}

            {draft.length === 0 ? (
                <p className="text-zinc-500">Chưa có thẻ RFID nào.</p>
            ) : (
                <table className="w-full border-collapse text-sm">
                    <thead>
                        <tr className="border-b border-black/[.12] text-left">
                            <th className="py-2 pr-4 font-medium">UID</th>
                            <th className="py-2 pr-4 font-medium">Email</th>
                            <th className="py-2 font-medium">Thao tác</th>
                        </tr>
                    </thead>
                    <tbody>
                        {draft.map((card) => (
                            <tr key={card.uid} className="border-b border-black/[.06]">
                                <td className="py-2 pr-4 font-mono">{card.uid}</td>
                                <td className="py-2 pr-4">
                                    {editingUid === card.uid ? (
                                        <input
                                            type="email"
                                            value={card.email}
                                            onChange={(e) => updateEmail(card.uid, e.target.value)}
                                            placeholder="Nhập email"
                                            className="w-full rounded-md border border-black/[.12] px-2 py-1"
                                        />
                                    ) : (
                                        <span className={card.email ? '' : 'text-zinc-400'}>
                                            {card.email || '—'}
                                        </span>
                                    )}
                                </td>
                                <td className="py-2">
                                    {editingUid === card.uid ? (
                                        <button
                                            type="button"
                                            onClick={finishEdit}
                                            className="rounded-md border border-black/[.12] px-3 py-1 text-xs font-medium transition-colors hover:bg-black/[.04]"
                                        >
                                            Xong
                                        </button>
                                    ) : (
                                        <button
                                            type="button"
                                            onClick={() => startEdit(card)}
                                            className="rounded-md border border-black/[.12] px-3 py-1 text-xs font-medium transition-colors hover:bg-black/[.04]"
                                        >
                                            Sửa
                                        </button>
                                    )}
                                </td>
                            </tr>
                        ))}
                    </tbody>
                </table>
            )}
        </div>
    );
}
