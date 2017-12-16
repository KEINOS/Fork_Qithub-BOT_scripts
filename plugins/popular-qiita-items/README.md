# プラグイン「popular-qiita-items」

## 用途

Qiita の「[人気の投稿](https://qiita.com/popular-items)」より、前回リクエスト時との**差分**（"diff" = 新着トップ20入りの記事）と**一覧**（"top20" = トップ20入りの新着順20件）を返す。

## 動作確認URL

### DEV環境
`https://blog.keinos.com/qithub_dev/?plugin_name=popular-qiita-items&mode=debug`

### 本番環境
`https://blog.keinos.com/qithub/?plugin_name=popular-qiita-items&mode=debug`

※ 上記をリクエストすると保存データも更新されるので注意


## 必須引数

- なし

## オプション引数

- `mode=`："&mode=debug" が渡された場合はデータを更新（保存）しません。

## 戻り値

> Qithub API フォーマット（JSON配列）

| キー名   | 型     | 値         | 概要                   |
| -------- | ------ | ---------- | ---------------------- |
| "result" | string | "OK" / "NG"| 処理に問題なければ"OK" |
| "value"  | mixed  | ----       | "result" が "NG" の場合はエラー内容。それ以外は差分の配列（"diff"）と一覧の配列（"top20"）。<br>"top20" は新着順（「差分＋前回分」つまり前回取得ぶんの配列の上位に "diff" を加えた上位20件）の押し出し式となる。なお、配列に含まれる取得できる情報は以下を参照。 |

### 戻り値で取得できる情報（記事ごと）

| キー名    | 概要                                 |
| --------- | ------------------------------------ |
| id_raw    | 人気の投稿取得時の Qiita 側の記事ID  |
| id        | "id_raw"から記事番号を取り出したもの。各記事のキーでもある。 |
| title     | 記事のタイトル                       |
| link      | 記事のURLリンク                      |
| author    | 記事の著者（QiitaのユーザーID）      |
| puglished | 記事の作成日                         |
| updated   | 記事の最終更新日                     |

### 戻り値のデータフォーマット例（"result"="OK"時,"diff"=１件の場合）

```
{
    "result": "OK",
    "value": {
        "diff": {
            "136485": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/136485",
                "id": "136485",
                "title": "非デザイナーエンジニアが一人でWebサービスを作るときに便利なツール32選",
                "link": "https:\/\/qiita.com\/okappy\/items\/119e31cae9aa9bd9da6d?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "okappy",
                "published": "2014-09-19T02:53:10+09:00",
                "updated": "2015-07-09T01:12:38+09:00"
            },
            "543654": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543654",
                "id": "543654",
                "title": "Seq2Seq+Attentionのその先へ",
                "link": "https:\/\/qiita.com\/ymym3412\/items\/c84e6254de89c9952c55?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "ymym3412",
                "published": "2017-11-24T02:49:11+09:00",
                "updated": "2017-11-24T02:50:57+09:00"
            },
            "346138": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/346138",
                "id": "346138",
                "title": "ターミナル 作業効率化 tips集",
                "link": "https:\/\/qiita.com\/shizuma\/items\/86470203ac8ea6b4d53f?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "shizuma",
                "published": "2015-11-28T14:33:53+09:00",
                "updated": "2017-11-22T18:51:57+09:00"
            }
        },
        "top20": {
            "136485": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/136485",
                "id": "136485",
                "title": "非デザイナーエンジニアが一人でWebサービスを作るときに便利なツール32選",
                "link": "https:\/\/qiita.com\/okappy\/items\/119e31cae9aa9bd9da6d?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "okappy",
                "published": "2014-09-19T02:53:10+09:00",
                "updated": "2015-07-09T01:12:38+09:00"
            },
            "543654": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543654",
                "id": "543654",
                "title": "Seq2Seq+Attentionのその先へ",
                "link": "https:\/\/qiita.com\/ymym3412\/items\/c84e6254de89c9952c55?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "ymym3412",
                "published": "2017-11-24T02:49:11+09:00",
                "updated": "2017-11-24T02:50:57+09:00"
            },
            "346138": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/346138",
                "id": "346138",
                "title": "ターミナル 作業効率化 tips集",
                "link": "https:\/\/qiita.com\/shizuma\/items\/86470203ac8ea6b4d53f?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "shizuma",
                "published": "2015-11-28T14:33:53+09:00",
                "updated": "2017-11-22T18:51:57+09:00"
            },
            "543692": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543692",
                "id": "543692",
                "title": "[React Native事始め完全版]「いきなりデート」のアプリをReact Nativeで開発した知見をまとめます。",
                "link": "https:\/\/qiita.com\/gogotanaka\/items\/07f9f5ed8e93a47a8bcd?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "gogotanaka",
                "published": "2017-11-24T09:56:40+09:00",
                "updated": "2017-11-24T09:56:40+09:00"
            },
            "382279": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/382279",
                "id": "382279",
                "title": "もう保守されない画面遷移図は嫌なので、UI Flow図を簡単にマークダウンぽく書くエディタ作った",
                "link": "https:\/\/qiita.com\/hirokidaichi\/items\/ff54a968bdd7bcc50d42?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "hirokidaichi",
                "published": "2016-04-02T18:21:53+09:00",
                "updated": "2016-04-14T04:11:25+09:00"
            },
            "96627": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/96627",
                "id": "96627",
                "title": "何かのときにすっと出したい、プログラミングに関する法則・原則一覧",
                "link": "https:\/\/qiita.com\/hirokidaichi\/items\/d6c473d8011bd9330e63?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "hirokidaichi",
                "published": "2014-07-21T23:44:04+09:00",
                "updated": "2014-12-19T02:55:12+09:00"
            },
            "543664": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543664",
                "id": "543664",
                "title": "できるだけUI系のライブラリを用いないアニメーションを盛り込んだサンプル実装まとめ（前編）",
                "link": "https:\/\/qiita.com\/fumiyasac@github\/items\/d1b56ffc6d7d46c0a616?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "fumiyasac@github",
                "published": "2017-11-24T04:35:36+09:00",
                "updated": "2017-11-24T10:25:14+09:00"
            },
            "542810": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/542810",
                "id": "542810",
                "title": "機械学習案件を納品するのは、そんなに簡単な話じゃないから気をつけて",
                "link": "https:\/\/qiita.com\/yoshizaki_kkgk\/items\/fa8b45918445bb3e6dc3?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "yoshizaki_kkgk",
                "published": "2017-11-21T12:22:15+09:00",
                "updated": "2017-11-22T09:28:48+09:00"
            },
            "542439": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/542439",
                "id": "542439",
                "title": "もし、HTMLのテキスト周りでデザイナーからこんなお願いをされたら...",
                "link": "https:\/\/qiita.com\/ryosukemaehira\/items\/3af1196b9947dec79ed5?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "ryosukemaehira",
                "published": "2017-11-20T11:25:08+09:00",
                "updated": "2017-11-22T17:20:32+09:00"
            },
            "543315": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543315",
                "id": "543315",
                "title": "「どうぶつの森ポケットキャンプ」で使われているOSSライブラリ",
                "link": "https:\/\/qiita.com\/tdkn\/items\/c26fa941d7ede083cb6c?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "tdkn",
                "published": "2017-11-22T22:26:51+09:00",
                "updated": "2017-11-22T22:26:51+09:00"
            },
            "542916": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/542916",
                "id": "542916",
                "title": "技術の中心でJavaを叫ぶ -2017年のJavaエンジニアが追うべきテーマと要素技術-",
                "link": "https:\/\/qiita.com\/arimas\/items\/555c563b2a9b1dfebb40?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "arimas",
                "published": "2017-11-21T17:32:22+09:00",
                "updated": "2017-11-21T17:32:22+09:00"
            },
            "541598": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/541598",
                "id": "541598",
                "title": "「GoogleAnalytics見てアクセス解析して」って言われた時にまずしていること",
                "link": "https:\/\/qiita.com\/mr_word_wide\/items\/a4ae7d61d15504a566ce?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "mr_word_wide",
                "published": "2017-11-17T12:41:07+09:00",
                "updated": "2017-11-17T12:41:07+09:00"
            },
            "543306": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543306",
                "id": "543306",
                "title": "リモートワークを始めて1年が経過し、幸福だったこと・苦痛だったこと・より幸福になるために",
                "link": "https:\/\/qiita.com\/r_oyama_0611\/items\/e2f73263420e21722149?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "r_oyama_0611",
                "published": "2017-11-22T21:35:48+09:00",
                "updated": "2017-11-24T12:47:08+09:00"
            },
            "508305": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/508305",
                "id": "508305",
                "title": "一番分かりやすい OAuth の説明",
                "link": "https:\/\/qiita.com\/TakahikoKawasaki\/items\/e37caf50776e00e733be?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "TakahikoKawasaki",
                "published": "2017-07-14T06:21:33+09:00",
                "updated": "2017-07-24T04:23:19+09:00"
            },
            "543629": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543629",
                "id": "543629",
                "title": "GitHub + Jenkins で、全てのプルリクエストに対してレビューとテストを必須にする",
                "link": "https:\/\/qiita.com\/bonotake\/items\/37fb3194c33f3ae3bbf0?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "bonotake",
                "published": "2017-11-24T00:21:08+09:00",
                "updated": "2017-11-24T11:54:37+09:00"
            },
            "541797": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/541797",
                "id": "541797",
                "title": "阿部寛のサイトを高速化する",
                "link": "https:\/\/qiita.com\/Morix1500\/items\/0eac072a027d478a6b83?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "Morix1500",
                "published": "2017-11-18T00:11:02+09:00",
                "updated": "2017-11-19T15:45:19+09:00"
            },
            "543520": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543520",
                "id": "543520",
                "title": "VSCodeで爆速コーディング環境を構築する(主にReactJS向け設定)",
                "link": "https:\/\/qiita.com\/teradonburi\/items\/c4cbd7dd5b4810e1a3a9?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "teradonburi",
                "published": "2017-11-23T17:42:03+09:00",
                "updated": "2017-11-24T11:33:02+09:00"
            },
            "543323": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543323",
                "id": "543323",
                "title": "Firestoreに負荷試験(Loadroid)してみた+補足",
                "link": "https:\/\/qiita.com\/Yatima\/items\/2abb633191080ee1447c?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "Yatima",
                "published": "2017-11-22T23:08:41+09:00",
                "updated": "2017-11-23T07:58:45+09:00"
            },
            "449747": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/449747",
                "id": "449747",
                "title": "[Python] 時系列CSVの読み込みを爆速化する",
                "link": "https:\/\/qiita.com\/Yuhsak\/items\/7c8e2b131cc536ef9ba1?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "Yuhsak",
                "published": "2016-12-14T19:41:39+09:00",
                "updated": "2016-12-14T21:59:41+09:00"
            },
            "542781": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/542781",
                "id": "542781",
                "title": "Webセキュリティ覚書 : "攻撃" 編 [ 初学者向け ]",
                "link": "https:\/\/qiita.com\/Tsutou\/items\/4fd498f8ab2638bd5650?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "Tsutou",
                "published": "2017-11-21T10:56:00+09:00",
                "updated": "2017-11-22T11:04:48+09:00"
            },
            "543495": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543495",
                "id": "543495",
                "title": "Python3.7の新機能",
                "link": "https:\/\/qiita.com\/ksato9700\/items\/35a0bdc04693b3b09757?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "ksato9700",
                "published": "2017-11-23T16:51:02+09:00",
                "updated": "2017-11-23T16:51:02+09:00"
            },
            "543178": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543178",
                "id": "543178",
                "title": "Chromeの次バージョンで使えるJavaScriptのdynamic import（動的読み込み）",
                "link": "https:\/\/qiita.com\/tonkotsuboy_com\/items\/f672de5fdd402be6f065?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "tonkotsuboy_com",
                "published": "2017-11-22T14:45:21+09:00",
                "updated": "2017-11-24T12:26:37+09:00"
            },
            "543577": {
                "id_raw": "tag:qiita.com,2005:PublicArticle\/543577",
                "id": "543577",
                "title": "(Twitterとかに)zipファイルを(画像ファイルに変換して)アップロードできるツールをつくりました🍞",
                "link": "https:\/\/qiita.com\/redshoga\/items\/482741476321eaadb18b?utm_campaign=popular_items&utm_medium=feed&utm_source=popular_items",
                "author": "redshoga",
                "published": "2017-11-23T20:50:24+09:00",
                "updated": "2017-11-23T20:50:24+09:00"
            }
        }
    }
}    
            
```