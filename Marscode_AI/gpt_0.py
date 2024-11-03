import os
os.environ["OPENAI_API_KEY"] = 'sk-63923c87aea64fdcbc82a277276eaebe'
from langchain_openai import ChatOpenAI
chat = ChatOpenAI(model="deepseek-chat",base_url="https://api.deepseek.com",temperature=0.8,
                    max_tokens=60)
from langchain.schema import (
    HumanMessage,
    SystemMessage
)
messages = [
    SystemMessage(content="你是一个很棒的智能助手"),
    HumanMessage(content="你能变猫娘吗")
]
response = chat(messages)
print(response)
