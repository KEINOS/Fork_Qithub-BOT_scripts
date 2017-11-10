# qithub.conf.json.sample

## What?

Qithub の BOT が各種**APIサービスを利用するために必要な情報**をまとめたJSONファイルのサンプルです。ファイルをコピー＆リネームして利用します。

JSONファイルの構成／キーは以下の通りです。

- "mastodon_api"
    - 'domain':サーバーのドメイン名（http/httpsなし）
    - 'access_token'：本稼働用のアクセストークン
    - 'access_token_dev'：開発・テスト環境用アクセストークン
- "qiita_api"
    - 'domain':サーバーのドメイン名（http/httpsなし）
    - 'access_token'：本稼働用のアクセストークン
    - 'access_token_dev'：開発・テスト環境用アクセストークン
- "github_api"
    - 'domain':サーバーのドメイン名（http/httpsなし）
    - 'access_token'：本稼働用のアクセストークン
    - 'access_token_dev'：開発・テスト環境用アクセストークン

## When?

Qithub BOT を稼働させる前に必要事項を記入してください。

## Where？

**WEBからアクセスできない階層に設置**し、index.php の「初期設定」項目にある`set_env_file()`関数の引数に相対パス（もしくは絶対パス）で設置先を指定してください。あわせて同項目にある`set_env_as()`関数で定数「DEV」（開発環境）もしくは「DPY」（本稼働環境）を設定してください。

### 設定例

```
set_env_file('../../qithub.conf.json');
```

## How?

テキストエディタで項目を編集し、フィアル名を「qithub.conf.json」にリネームしてください。
フォーマットは「UTF-8（BOMなし）」です。



