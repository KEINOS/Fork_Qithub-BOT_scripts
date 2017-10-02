# hello_world.php

 'Hello world!'をトゥートして、前回のトゥートを削除するだけのスクリプト
 
| プログラム言語 | 呼び出しスクリプト名 | issue番号 |
| --- | --- | :---: |
| PHP | hello_world.php | [issue #10](https://github.com/Qithub-BOT/scripts/issues/10) |

## 動作実績

| 言語バージョン | サーバー |
| --- | --- |
| PHP 7.0.22 (cli) | Xrea @ ValueDomain |

## 動作仕様

1. スクリプト実行時に標準入力より以下の引数を取得
    - 引数1：URLエンコードされたJSONデータ<br>※ 引数に含まれる情報は次項の「標準入力仕様」を参照
1. 引数がない場合はエラーを返して終了する
1. 引数のうち"domain"（マストドンのホスト名）と"access_token"を取得
1. 同階層に"last_toot.dat"テキスト・ファイルがある場合は読み込む
    - "last_toot.dat"より前回のトゥートIDを取得し、トゥートを削除する
1. 現在の時刻（"Y/m/d H:i:s"のタイムスタンプ） を取得
1. タイムスタンプ付きで"Hello world!"をトゥートし、トゥートIDを取得
1. 同階層に"last_toot.dat"を作成し、serializeされたトゥートIDを保存
1. トゥート時のMastodonAPIからの返信内容を表示して終了

### 標準入力仕様

標準入力で受け取るJSONデータの内容。（要Qithub API仕様制定）
これらの配列データがURLエンコードされ、第１引数に渡されます。

| キー | 型 | 概要 | 用途 |
| --- | --- | --- | --- |
| is_mode_debug | bool | デバッグモードのフラグ | BOTの呼び出しURLに"mode=debug"が含まれている場合の動作用 |
| keys | array | 各種APIキー | トゥートに必要な Mastodon ホスト名、アクセストークンなど。"scheme","domain","client_id","client_secret","access_token"が含まれる |
| get | array | URLのGETクエリ情報 | BOTの呼び出しURLに含まれるGETヘッダーの内容 |
| post | array | POST情報 | BOTの呼び出し時にPOSTされたヘッダー情報 |
